#pragma once

#include "Callback.hpp"
#include <QJSValue>
#include <QVariant>
#include <QQmlEngine>

namespace quickstreams {

class Stream;

class JsCallback : public Callback {
	friend class Stream;

protected:
	QJSValue _function;
	QQmlEngine* _engine;

	void execute(const QVariant& data);

public:
	JsCallback(QQmlEngine* engine, const QJSValue& function);
};

}
