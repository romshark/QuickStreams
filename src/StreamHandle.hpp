#pragma once

#include <functional>
#include <QObject>
#include <QString>
#include <QVariant>

namespace quickstreams {

class Stream;

class StreamHandle {
	friend class Stream;
	Q_GADGET
	Q_PROPERTY(bool isAbortable READ isAbortable)
	Q_PROPERTY(bool isAborted READ isAborted)

public:
	typedef std::function<void(const QString&, const QVariant&)> EventCallback;
	typedef std::function<void(const QVariant&)> CloseCallback;
	typedef std::function<void(const QVariant&)> FailCallback;
	typedef std::function<Stream*(const QVariant&)> AdoptCallback;
	typedef std::function<bool()> IsAbortableCallback;
	typedef std::function<bool()> IsAbortedCallback;
	typedef std::function<Stream*()> ReferenceGetter;

protected:
	EventCallback _eventCb;
	CloseCallback _closeCb;
	FailCallback _failCb;
	AdoptCallback _adoptCb;
	ReferenceGetter _refGet;
	IsAbortableCallback _isAbortableCb;
	IsAbortedCallback _isAbortedCb;

	Stream* reference() const;

public:
	StreamHandle();
	StreamHandle(
		EventCallback eventCb,
		CloseCallback closeCb,
		FailCallback failCb,
		AdoptCallback adoptCb,
		ReferenceGetter refGet,
		IsAbortableCallback isAbortableCb,
		IsAbortedCallback isAbortedCb
	);

	// Makes this stream immediately emit an event optionally passing any data.
	Q_INVOKABLE void event(
		const QString& name,
		const QVariant& data = QVariant()
	);

	// Immediately closes this stream optionally passing any data.
	Q_INVOKABLE void close(const QVariant& data = QVariant());

	// Immediately fails the stream optionally passing any data
	// describing the reason of failure.
	Q_INVOKABLE void fail(const QVariant& data = QVariant());

	// Adopts the passed stream making this stream become its parent
	// and returns the passed stream to allow for chaining
	Q_INVOKABLE Stream* adopt(const QVariant& target);

	bool isAbortable() const;
	bool isAborted() const;
};

} // quickstreams

Q_DECLARE_METATYPE(quickstreams::StreamHandle*)
