#include <eeros/hal/DummyLogicOutput.hpp>

using namespace eeros::hal;

DummyLogicOutput::DummyLogicOutput(std::string id) : Output<bool>(id) { }

bool DummyLogicOutput::get() {
	return false;
}

void DummyLogicOutput::set(bool value) {
	
}
