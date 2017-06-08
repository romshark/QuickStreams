#pragma once

#include "Repeater.hpp"
#include <QJSValue>

namespace quickstreams {

class JsRepeater : public Repeater {
protected:
	QJSValue _function;

public:
	JsRepeater(const QJSValue& function);
	bool evaluate(bool isAborted);
};

} // quickstreams

