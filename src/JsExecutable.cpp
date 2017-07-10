#include "JsExecutable.hpp"
#include "Executable.hpp"
#include "QmlStream.hpp"
#include "QmlStreamHandle.hpp"
#include "Error.hpp"
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
	auto var(result.toVariant());

	// Evaluate execution results. If a stream was returned then wrap it;
	// If error was returned then remember the error for later handling.
	if(var.canConvert<quickstreams::qml::QmlStream*>()) {
		_returnedStream = qjsvalue_cast<quickstreams::qml::QmlStream*>(
			result
		)->_reference.data();
	} else if(result.isError()) {
		auto exception(new quickstreams::exception::JsException(
			result.property("name").toString(),
			result.property("message").toString(),
			QVariant()
		));
		_engine->setObjectOwnership(exception, QQmlEngine::CppOwnership);
		_error = Error(qobject_cast<exception::JsException*>(exception));
	} else if(var.canConvert<exception::Exception*>()) {
		auto exception(var.value<exception::Exception*>());
		_error = Error(exception);
	}
}
