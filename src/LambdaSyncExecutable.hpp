#pragma once

#include "Executable.hpp"
#include "StreamHandle.hpp"
#include <functional>
#include <QVariant>
#include <QMetaType>

namespace quickstreams {

class Provider;
class Stream;

class LambdaSyncExecutable : public Executable {
	friend class Provider;
	friend class Stream;

public:
	typedef std::function<QVariant (const QVariant&)> Function;

protected:
	Function _function;

	LambdaSyncExecutable(Function function);

public:
	void execute(const QVariant& data);
};

} // quickstreams
