#include "Trigger.hpp"

Trigger::Trigger() :
	QObject(nullptr),
	_spy(this, &Trigger::triggered)
{
}

bool Trigger::wait(int timeout) {
	return _spy.wait(timeout);
}

int Trigger::count() const {
	return _spy.count();
}

void Trigger::trigger() {
	triggered();
}
