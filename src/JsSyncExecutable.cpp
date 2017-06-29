#include "JsSyncExecutable.hpp"
#include "Executable.hpp"
#include "QmlStream.hpp"
#include "QmlStreamHandle.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>

quickstreams::qml::JsSyncExecutable::JsSyncExecutable(
	QQmlEngine* engine,
	const QJSValue& function
) :
	_engine(engine),
	_function(function)
{}

void quickstreams::qml::JsSyncExecutable::execute(const QVariant& data) {
	// Execute
	QJSValue result(_function.call({_engine->toScriptValue(data)}));

	// Evaluate execution results. If a stream was returned then wrap it;
	// if an error was returned then remember the error for later handling;
	// if anything else is returned (any value of any type or null/void)
	// then close the stream referenced by the handle immediately.
	if(result.toVariant().canConvert<quickstreams::qml::QmlStream*>()) {
		_returnedStream = qjsvalue_cast<quickstreams::qml::QmlStream*>(
			result
		)->_reference.data();
	} else if(result.isError()) {
		_error.reset(new QVariant(result.toVariant()));
	} else {
		_handle->close(QVariant(result.toVariant()));
	}
}
