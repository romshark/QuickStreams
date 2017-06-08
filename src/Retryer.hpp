#pragma once

#include <QVariant>
#include <QVariantList>
#include <QSharedPointer>

namespace quickstreams {

class Retryer {
public:
	typedef QSharedPointer<Retryer> Reference;

protected:
	QVariantList _errorSamples;
	qint32 _maxTrials;
	qint32 _currentTrial;

public:
	Retryer(const QVariantList& _errorSamples, qint32 maxTrials);
	void reset();
	void incrementTrialCounter();
	bool isInfinite() const;
	bool verifyErrorListed(const QVariant& error) const;
	bool verify(const QVariant& error);
};

} // quickstreams
