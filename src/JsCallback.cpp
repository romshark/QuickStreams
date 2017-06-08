#include "JsCallback.hpp"
#include <QJSValue>
#include <QVariant>
#include <QQmlEngine>

quickstreams::JsCallback::JsCallback(
	QQmlEngine* engine,
	const QJSValue& function
) :
	_function(function),
	_engine(engine)
{}

void quickstreams::JsCallback::execute(const QVariant& data) {
	_function.call({
		_engine->toScriptValue(data)
	});
}
