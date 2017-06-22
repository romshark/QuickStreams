#pragma once

#include "Executable.hpp"
#include "QmlStreamHandle.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
#include <QMetaType>

namespace quickstreams {
namespace qml {

class QmlStream;
class QmlProvider;

class JsExecutable : public Executable {
	friend class QmlStream;
	friend class QmlProvider;

protected:
	QQmlEngine* _engine;
	QmlStreamHandle _qmlHandle;
	QJSValue _function;

	JsExecutable(
		QQmlEngine* engine,
		const QJSValue& function
	);

public:
	void execute(const QVariant& data);
};

}} // quickstreams::qml
