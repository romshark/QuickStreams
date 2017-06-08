#pragma once

#include "Repeater.hpp"
#include <functional>

namespace quickstreams {

class LambdaRepeater : public Repeater {
public:
	typedef std::function<bool(bool)> Function;

protected:
	Function _function;

public:
	LambdaRepeater(Function function);
	bool evaluate(bool isAborted);
};

} // quickstreams

