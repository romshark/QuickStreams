#include "Retryer.hpp"
#include "JsConditionRetryer.hpp"
#include <QQmlEngine>
#include <QVariant>
#include <QJSValue>

quickstreams::JsConditionRetryer::JsConditionRetryer(
	QQmlEngine* engine, const QJSValue& function, qint32 maxTrials
) :
	Retryer(maxTrials),
	_engine(engine),
	_function(function)
{}

bool quickstreams::JsConditionRetryer::verifyCondition(const QVariant& error) {
	if(!_function.isCallable()) return false;
	QJSValue result(_function.call({
		_engine->toScriptValue(error)
	}));
	if(!result.isBool()) return false;
	return result.toBool();
}
