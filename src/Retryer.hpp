#pragma once

#include <QVariant>
#include <QVariantList>
#include <QSharedPointer>

namespace quickstreams {

class Retryer {
public:
	typedef QSharedPointer<Retryer> Reference;

protected:
	qint32 _maxTrials;
	qint32 _currentTrial;

public:
	Retryer(qint32 maxTrials);
	void reset();
	bool isInfinite() const;
	bool isMaxReached() const;
	bool verify(const QVariant& error);

	virtual bool verifyCondition(const QVariant& error) = 0;
};

} // quickstreams
