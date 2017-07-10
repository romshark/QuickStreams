#pragma once

#include "Retryer.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>

namespace quickstreams {

class JsConditionRetryer : public Retryer {
protected:
	QQmlEngine* _engine;
	QJSValue _function;

public:
	JsConditionRetryer(
		QQmlEngine* engine,
		const QJSValue& function,
		qint32 maxTrials
	);
	bool verifyCondition(const QVariant& error);
};

} // quickstreams
