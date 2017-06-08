#pragma once

#include "Callback.hpp"
#include <functional>
#include <QVariant>

namespace quickstreams {

class Stream;

class LambdaCallback : public Callback {
	friend class Stream;

public:
	typedef std::function<void(const QVariant&)> Function;

protected:
	Function _function;

	void execute(const QVariant& data);

public:
	LambdaCallback(Function function);
};

}
