#ifndef MYSAFETYPROPERTIES_HPP_
#define MYSAFETYPROPERTIES_HPP_

#include <eeros/safety/SafetyProperties.hpp>
#include <eeros/hal/HAL.hpp>
#include "MyControlSystem.hpp"

class MySafetyProperties : public eeros::safety::SafetyProperties {
	
public:
	MySafetyProperties(MyControlSystem& controlSys);
	virtual ~MySafetyProperties();
	
protected:
	// critical outputs
	eeros::hal::Output<bool>* enable;
	
	// critical inputs
	eeros::hal::Input<bool>* emergency;
	eeros::hal::Input<double>* q;
		
	MyControlSystem& controlSys;
	
	// Define all possible events
	eeros::safety::SafetyEvent doSystemOn;
	eeros::safety::SafetyEvent doSystemOff;
	eeros::safety::SafetyEvent startControl;
	eeros::safety::SafetyEvent stopControl;
	eeros::safety::SafetyEvent startControlDone;
	eeros::safety::SafetyEvent stopControlDone;
	eeros::safety::SafetyEvent startMoving;
	eeros::safety::SafetyEvent stopMoving;
	eeros::safety::SafetyEvent doEmergency;
	eeros::safety::SafetyEvent resetEmergency;
	
	// Name all levels
	eeros::safety::SafetyLevel off;
	eeros::safety::SafetyLevel emergencyState;
	eeros::safety::SafetyLevel systemOn;
	eeros::safety::SafetyLevel startingControl;
	eeros::safety::SafetyLevel stoppingControl;
	eeros::safety::SafetyLevel powerOn;
	eeros::safety::SafetyLevel moving;
};

#endif // MYSAFETYPROPERTIES_HPP_
