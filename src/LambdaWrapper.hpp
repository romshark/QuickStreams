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

class LambdaWrapper : public Executable {
	friend class Provider;
	friend class Stream;

	typedef QSharedPointer<Stream> StreamReference;

public:
	typedef std::function<StreamReference (const QVariant&)> Function;

protected:
	Function _function;

	LambdaWrapper(Function function);

public:
	void execute(const QVariant& data);
};

} // quickstreams
