#include "Retryer.hpp"
#include <QVariant>
#include <QVariantList>

quickstreams::Retryer::Retryer(qint32 maxTrials) :
	_maxTrials(maxTrials),
	_currentTrial(0)
{}

void quickstreams::Retryer::Retryer::reset() {
	_currentTrial = 0;
}

bool quickstreams::Retryer::isInfinite() const {
	return _maxTrials < 0;
}

bool quickstreams::Retryer::isMaxReached() const {
	return _currentTrial > _maxTrials;
}

bool quickstreams::Retryer::verify(const QVariant& error) {
	++_currentTrial;
	if((isInfinite() || !isMaxReached()) && verifyCondition(error)) {
		return true;
	}
	return false;
}
