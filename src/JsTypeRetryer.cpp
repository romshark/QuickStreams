#include "Retryer.hpp"
#include "JsTypeRetryer.hpp"
#include "Error.hpp"
#include <QVariant>
#include <QVariantList>
#include <QMetaType>

quickstreams::JsTypeRetryer::JsTypeRetryer(
	const QVariant& errorTypes, qint32 maxTrials
) :
	Retryer(maxTrials)
{
	switch(errorTypes.userType()) {
	case QMetaType::Int: _ids.insert(errorTypes.toInt()); break;
	case QMetaType::QString: _names.insert(errorTypes.toString()); break;
	case QMetaType::QVariantList:
		auto list(errorTypes.toList());
		for(
			QVariantList::const_iterator itr(list.constBegin());
			itr != list.constEnd();
			itr++
		) {
			// Accept only integer and string identifiers, ignore other types
			switch(itr->userType()) {
			case QMetaType::Int: _ids.insert(itr->toInt()); break;
			case QMetaType::QString: _names.insert(itr->toString()); break;
			default: break;
			}
		}
		break;
	}
}

bool quickstreams::JsTypeRetryer::verifyCondition(const QVariant& error) {
	auto err(error.value<Error>());
	auto type(err.type());

	// In case of typed errors check by integer metatype identifier
	if(type != exception::JsException::type()) return _ids.contains(type);

	// In case of JavaScript errors check by name
	auto jsError(err.as<exception::JsException>());
	return _names.contains(jsError->name());
}
