#include <eeros/sequencer/BaseSequence.hpp>
#include <eeros/core/Fault.hpp>
#include <unistd.h>

using namespace eeros;
using namespace eeros::sequencer;

BaseSequence::BaseSequence(Sequencer& seq, BaseSequence* caller) : 
	seq(seq), caller(caller), conditionTimeout(), monitorTimeout(this, conditionTimeout, SequenceProp::abort),
	state(SequenceState::idle), blocking(true), pollingTime(100), log('X')
{
	if (caller == nullptr) {
		static int numberOfMainSequence;
		numberOfMainSequence++;
		if (numberOfMainSequence > 1) 
			throw Fault("Only one main sequence is possible. Use 'Sequence(S, caller, name)' to construct a normal sequence");
		else 
			isMainSequence = true;
	}
	if (!isMainSequence) {	// get and update callerStack
		callerStack = caller->getCallerStack();
		callerStack.push_back(caller);	// add latest caller
	}
	addMonitor(&monitorTimeout);	// default monitor
}

BaseSequence::~BaseSequence() { }

int BaseSequence::action() {
	state = SequenceState::running;
	checkActiveException();	// check if this or a caller sequence is 'exceptionIsActive', set state accordingly

	if ((caller != NULL) && (state == SequenceState::running) && (caller->state == SequenceState::restarting)) 
		state = SequenceState::aborting;
	
	do {	//for restarting
		if (state == SequenceState::restarting) {	// sequence got restarted
			log.info() << "restart sequence '" << name << "'";
			state = SequenceState::running;
// 			sequenceIsRestarting = false;
			resetTimeout();
// 			restartCounter++;
		} //else restartCounter = 0;
		bool firstCheck = true;
		if (checkPreCondition()) {
			checkMonitorsOfBlockedCallers();	// check for monitors of all callers, start exception sequence, mark owner of fired monitor with 'exceptionIsActive'
// 			log.fatal();
			checkActiveException();		// check if this or a caller sequence is 'exceptionIsActive', set state accordingly
			if (state == SequenceState::running) {
				log.info() << "start '" << name <<"'";
				action();	// call to custom implementation of method
			}
			while (state == SequenceState::running) {
				if (!firstCheck) {
					if (checkExitCondition()) state = SequenceState::terminated;	// check exit condition
					if (state != SequenceState::terminated) {
						checkMonitorsOfThisSequence();		// sets activeException if needed
						checkMonitorsOfBlockedCallers();	// sets activeException if needed
					}
				} else firstCheck = false;
				
				checkActiveException();					// sets state according to activeException
				if (state == SequenceState::running) usleep(pollingTime*1000);
			}
		} else { state = SequenceState::terminated;}
	} while (state == SequenceState::restarting);
	
	if (state == SequenceState::aborting) state = SequenceState::aborted;
	else state = SequenceState::terminated;
}

void BaseSequence::addMonitor(Monitor* monitor) {monitors.push_back(monitor);}

std::vector<Monitor*> BaseSequence::getMonitors() const {return monitors;}

void BaseSequence::checkMonitorsOfThisSequence() {
	for (Monitor* monitor : getMonitors()) checkMonitor(monitor);
}

void BaseSequence::checkMonitorsOfBlockedCallers() {
	if (!callerStackCreated) {	// when first called, caller stack has to be created
		if (this->blocking) {	// nonblocking sequences have no blocking callers
			std::vector<BaseSequence*> tempStack;
			log.trace() << "caller stack of seq " << name;
			for (int i = callerStack.size() - 1; i >= 0; i--) {	// reverse iteration
				BaseSequence* entry = callerStack[i];
				log.trace() << "  entry " << entry->getName();
				tempStack.push_back(entry);
				if (!entry->isBlocking()) break;	// stop, if caller is nonblocking
			}
			log.trace() << "caller stack blocking of seq " << name;
			for (int i = tempStack.size(); i--;) {		// reverse vector
				callerStackBlocking.push_back(tempStack[i]);
				log.trace() << "  entry " << tempStack[i]->getName();
			}
		}
		callerStackCreated = true;
	}
	for (BaseSequence* s : callerStackBlocking) {
		if (!s->inExcProcessing) 
			for (Monitor* m : s->getMonitors()) checkMonitor(m);
	}
}

void BaseSequence::checkMonitor(Monitor* monitor) {
	if (monitor->checkCondition() == true) {
		if (!monitor->getOwner()->exceptionIsActive) {
			log.info() << "monitor fired";
			switch (monitor->getBehavior()) {
				case SequenceProp::resume : break;
				case SequenceProp::abort : 
				case SequenceProp::restart : monitor->getOwner()->setActiveException(monitor); break;
// 				case SequenceProp::abortCallerOfOwner :	
// 				case SequenceProp::restartCallerOfOwner : {
// 					BaseSequence* seq = monitor->getOwner()->getCallerSequence();
// 					if (seq != nullptr) seq->setActiveException(monitor); 
// 					else monitor->getOwner()->setActiveException(monitor);	// stop sequence if requested caller not present
// 					break;
// 				}
				default : break;
			}
			monitor->startExceptionSequence();	// start only if not yet in exception processing, blocking
			monitor->getOwner()->inExcProcessing = false;
		}
	}
}

void BaseSequence::setActiveException(Monitor* activeMonitor) {
	switch (activeMonitor->getBehavior()) {
		case SequenceProp::resume :
		case SequenceProp::abort :
		case SequenceProp::restart :
			activeMonitor->getOwner()->exceptionIsActive = true;
			activeMonitor->getOwner()->activeException = activeMonitor;
			break;
// 		case SequenceProp::abortCallerOfOwner :
// 		case SequenceProp::restartCallerOfOwner : {
// 			BaseSequence* seq = activeMonitor->getOwner()->getCallerSequence();
// 			if (seq != nullptr) {
// 				activeMonitor->getOwner()->getCallerSequence()->exceptionIsActive = true;
// 				activeMonitor->getOwner()->getCallerSequence()->activeException = activeMonitor;
// 			} else {
// 				activeMonitor->getOwner()->exceptionIsActive = true;
// 				activeMonitor->getOwner()->activeException = activeMonitor;
// 			}
// 			break;
// 		}
		default : break;
	}
}

void BaseSequence::clearActiveException() {
	exceptionIsActive = false;
	activeException = nullptr;
}

void BaseSequence::checkActiveException() {
	if (exceptionIsActive == true) {	// this sequence got the order to abort, restart ...
// 		log.warn() << "this";
		switch (activeException->getBehavior()) {
			case SequenceProp::resume : break;
			case SequenceProp::abort : state = SequenceState::aborting; break;
			case SequenceProp::restart : state = SequenceState::restarting; break;
			default : break;
		}
		clearActiveException();
	} else {				// a caller got the order to abort, restart ...
		for (BaseSequence* s : getCallerStack()) {
			if (s->exceptionIsActive && !s->inExcProcessing) {
// 				log.warn() << s->exceptionIsActive;
// 				state = SequenceState::aborting;
				switch (s->activeException->getBehavior()) {
					case SequenceProp::resume : break;
					case SequenceProp::abort : 
					case SequenceProp::restart : state = SequenceState::aborting; break;
					default : break;
				}
			}
		}
	}
}

// bool BaseSequence::isStep() const {return false;}

bool BaseSequence::checkExitCondition() {return true;}

bool BaseSequence::checkPreCondition() {return true;}

void BaseSequence::setName(std::string name) {this->name = name;}

std::string BaseSequence::getName() const {return name;}

SequenceState BaseSequence::getRunningState() const {return state;}

void BaseSequence::setId(int id) {this->id = id;}

int BaseSequence::getId() const {return id;}

bool BaseSequence::isBlocking() const {return blocking;}

BaseSequence* BaseSequence::getCallerSequence() {
	if (caller == nullptr) {
		log.error() << "'" << name << "' does not have a caller";
		return nullptr;
	}
	else return caller;
}

std::vector<BaseSequence*> BaseSequence::getCallerStack() const {return callerStack;}

// void BaseSequence::restartSequence() {
// 	state = SequenceState::restarting;
// 	sequenceIsRestarting = true;
// }

void BaseSequence::setPollingTime(int timeInMilliseconds) {
	pollingTime = timeInMilliseconds;
}

void BaseSequence::setTimeoutTime(double timeoutInSec) {
	conditionTimeout.setTimeoutTime(timeoutInSec);
}

void BaseSequence::resetTimeout() {
	conditionTimeout.resetTimeout();
}

void BaseSequence::setTimeoutBehavior(SequenceProp behavior) {
	monitorTimeout.setBehavior(behavior);
}

void BaseSequence::setTimeoutExceptionSequence(BaseSequence& sequence) {
	monitorTimeout.setExceptionSequence(sequence);
}