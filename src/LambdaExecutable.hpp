#pragma once

#include "Executable.hpp"
#include "StreamHandle.hpp"
#include <functional>
#include <QVariant>
#include <QMetaType>
#include <QSharedPointer>

namespace quickstreams {

class Provider;
class Stream;

class LambdaExecutable : public Executable {
	friend class Provider;
	friend class Stream;

	typedef QSharedPointer<Stream> Reference;

public:
	typedef std::function<
		QSharedPointer<Stream> (const StreamHandle&, const QVariant&)
	> Function;

protected:
	Function _function;

	LambdaExecutable(Function function);

public:
	void execute(const QVariant& data);
};

} // quickstreams
