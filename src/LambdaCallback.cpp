#include "LambdaCallback.hpp"
#include <QVariant>

quickstreams::LambdaCallback::LambdaCallback(Function function) :
	_function(function)
{}

void quickstreams::LambdaCallback::execute(const QVariant& data) {
	_function(data);
}
