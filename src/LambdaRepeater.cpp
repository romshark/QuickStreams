#include "LambdaRepeater.hpp"

quickstreams::LambdaRepeater::LambdaRepeater(Function function) :
	_function(function)
{}

bool quickstreams::LambdaRepeater::evaluate(bool isAborted) {
	try {
		return _function(isAborted);
	} catch(...) {
		return false;
	}
}
