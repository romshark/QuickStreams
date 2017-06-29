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

class JsSyncExecutable : public Executable {
	friend class QmlStream;
	friend class QmlProvider;

protected:
	QQmlEngine* _engine;
	QmlStreamHandle _qmlHandle;
	QJSValue _function;

	JsSyncExecutable(
		QQmlEngine* engine,
		const QJSValue& function
	);

public:
	void execute(const QVariant& data);
};

}} // quickstreams::qml
