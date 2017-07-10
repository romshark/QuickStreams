#pragma once

#include "Retryer.hpp"
#include "LambdaRetryer.hpp"
#include <functional>
#include <QVariant>

namespace quickstreams {

class LambdaRetryer : public Retryer {
public:
	typedef std::function<bool (const QVariant&)> Function;

protected:
	Function _function;

public:
	LambdaRetryer(Function function, qint32 maxTrials = -1);
	bool verifyCondition(const QVariant& error);
};

} // quickstreams
