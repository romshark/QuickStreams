#include "Retryer.hpp"
#include "TypeRetryer.hpp"
#include "Error.hpp"
#include <QVariant>
#include <QVariantList>

quickstreams::TypeRetryer::TypeRetryer(
	const TypeList& errorTypes, qint32 maxTrials
) :
	Retryer(maxTrials),
	_errorTypes(errorTypes)
{}

bool quickstreams::TypeRetryer::verifyCondition(const QVariant& error) {
	return _errorTypes.contains(error.value<Error>().type());
}
