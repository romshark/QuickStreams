#include "Retryer.hpp"
#include <QVariant>
#include <QVariantList>

quickstreams::Retryer::Retryer(
	const QVariantList& errorSamples, qint32 maxTrials
) :
	_errorSamples(errorSamples),
	_maxTrials(maxTrials),
	_currentTrial(0)
{}

void quickstreams::Retryer::Retryer::reset() {
	_currentTrial = 0;
}

void quickstreams::Retryer::incrementTrialCounter() {
	++_currentTrial;
}

bool quickstreams::Retryer::isInfinite() const {
	return _maxTrials < 0;
}

bool quickstreams::Retryer::verifyErrorListed(const QVariant& error) const {
	for(
		QVariantList::const_iterator itr(_errorSamples.constBegin());
		itr != _errorSamples.constEnd();
		itr++
	) {
		if(error == *itr) return true;
	}
	return false;
}

bool quickstreams::Retryer::verify(const QVariant& error) {
	if(
		(_maxTrials < 0 || _currentTrial <= _maxTrials)
		&& verifyErrorListed(error)
	) {
		return true;
	}
	return false;
}
