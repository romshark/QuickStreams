#pragma once

#include "StreamHandle.hpp"
#include <QVariant>
#include <QSharedPointer>

namespace quickstreams {

class Stream;

class Executable {
	friend class Stream;

public:
	typedef QSharedPointer<Executable> Reference;

protected:
	StreamHandle* _handle;
	QSharedPointer<QVariant> _error;
	Stream* _returnedStream;

	void setHandle(StreamHandle* handle);

public:
	virtual ~Executable() {}
	Executable();
	void reset();
	bool hasFailed() const;
	bool hasReturnedStream() const;
	QVariant getError() const;
	Stream* stream();

	virtual void execute(const QVariant& data) = 0;
};

} // quickstreams

Q_DECLARE_OPAQUE_POINTER(quickstreams::Stream)
