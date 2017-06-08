#pragma once

#include <functional>
#include <QString>
#include <QVariant>
#include <QSharedPointer>

namespace quickstreams {

class Stream;

namespace qml {

class QmlStreamHandle;

}

class StreamHandle {
	friend class Stream;
	friend class qml::QmlStreamHandle;

public:
	typedef QSharedPointer<Stream> StreamReference;

	typedef std::function<void(const QString&, const QVariant&)> EventCallback;
	typedef std::function<void(const QVariant&)> CloseCallback;
	typedef std::function<void(const QVariant&)> FailCallback;
	typedef std::function<StreamReference(StreamReference&)> AdoptCallback;
	typedef std::function<bool()> IsAbortableCallback;
	typedef std::function<bool()> IsAbortedCallback;

protected:
	EventCallback _eventCb;
	CloseCallback _closeCb;
	FailCallback _failCb;
	AdoptCallback _adoptCb;
	IsAbortableCallback _isAbortableCb;
	IsAbortedCallback _isAbortedCb;

	StreamHandle(
		EventCallback eventCb,
		CloseCallback closeCb,
		FailCallback failCb,
		AdoptCallback adoptCb,
		IsAbortableCallback isAbortableCb,
		IsAbortedCallback isAbortedCb
	);

public:
	StreamHandle();

	// Makes this stream immediately emit an event optionally passing any data.
	void event(const QString& name, const QVariant& data = QVariant()) const;

	// Immediately closes this stream optionally passing any data.
	void close(const QVariant& data = QVariant()) const;

	// Immediately fails the stream optionally passing any data
	// describing the reason of failure.
	void fail(const QVariant& data = QVariant()) const;

	// Adopts the passed stream making this stream become its parent
	// and returns the passed stream to allow for chaining
	StreamReference adopt(StreamReference stream) const;

	bool isAbortable() const;
	bool isAborted() const;
};

} // quickstreams
