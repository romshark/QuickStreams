#include "JsExecutable.hpp"
#include "Executable.hpp"
#include "QmlStream.hpp"
#include "QmlStreamHandle.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>

quickstreams::qml::JsExecutable::JsExecutable(
	QQmlEngine* engine,
	const QJSValue& function
) :
	_engine(engine),
	_function(function)
{}

void quickstreams::qml::JsExecutable::execute(const QVariant& data) {
	// Execute
	QJSValue result(_function.call({
		_engine->toScriptValue(_qmlHandle),
		_engine->toScriptValue(data),
	}));

	// Evaluate execution results. If a stream was returned then wrap it;
	// If error was returned then remember the error for later handling.
	if(result.toVariant().canConvert<quickstreams::qml::QmlStream*>()) {
		_returnedStream = qjsvalue_cast<quickstreams::qml::QmlStream*>(
			result
		)->_reference.data();
	} else if(result.isError()) {
		_error.reset(new QVariant(QVariant::fromValue<QJSValue>(result)));
	}
}
