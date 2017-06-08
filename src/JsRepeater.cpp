#include "JsRepeater.hpp"
#include <QJSValue>

quickstreams::JsRepeater::JsRepeater(const QJSValue& function) :
	_function(function)
{}

bool quickstreams::JsRepeater::evaluate(bool isAborted) {
	QJSValue result(_function.call({QJSValue(isAborted)}));
	if(result.isBool()) return result.toBool();
	return false;
}
