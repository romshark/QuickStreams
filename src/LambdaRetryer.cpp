#include "Retryer.hpp"
#include "LambdaRetryer.hpp"
#include <QVariant>

quickstreams::LambdaRetryer::LambdaRetryer(
	Function function, qint32 maxTrials
) :
	Retryer(maxTrials),
	_function(function)
{}

bool quickstreams::LambdaRetryer::verifyCondition(const QVariant& error) {
	return _function(error);
}
