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
	Q_PROPERTY(bool isWrapper READ isWrapper)
	Q_PROPERTY(bool isAbortable READ isAbortable)
	Q_PROPERTY(bool isAborted READ isAborted)

public:
	typedef std::function<void(const QString&, const QVariant&)> EventCallback;
	typedef std::function<void(const QVariant&)> CloseCallback;
	typedef std::function<void(const QVariant&)> FailCallback;
	typedef std::function<bool(const QVariant&)> WrapCallback;
	typedef std::function<Stream*(const QVariant&)> AdoptCallback;
	typedef std::function<bool()> IsAbortableCallback;
	typedef std::function<bool()> IsAbortedCallback;
	typedef std::function<Stream*()> ReferenceGetter;

protected:
	bool _isWrapper;
	EventCallback _eventCb;
	CloseCallback _closeCb;
	FailCallback _failCb;
	WrapCallback _wrapCb;
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
		WrapCallback wrapCb,
		AdoptCallback adoptCb,
		ReferenceGetter refGet,
		IsAbortableCallback isAbortableCb,
		IsAbortedCallback isAbortedCb
	);

	// Makes this stream immediately emit an event optionally passing any data.
	//
	// NOTE: won't trigger if this stream wraps another stream.
	Q_INVOKABLE void event(
		const QString& name,
		const QVariant& data = QVariant()
	);

	// Immediately closes this stream optionally passing any data.
	//
	// NOTE: won't trigger if this stream wraps another stream.
	Q_INVOKABLE void close(const QVariant& data = QVariant());

	// Immediately fails the stream optionally passing any data
	// describing the reason of failure.
	//
	// NOTE: won't trigger if this stream wraps another stream.
	Q_INVOKABLE void fail(const QVariant& data = QVariant());

	// Wraps a stream attaching to its close and failure signals.
	Q_INVOKABLE void wrap(const QVariant& data);

	// Adopts the passed stream making this stream become its parent
	// and returns the passed stream to allow for chaining
	//
	// NOTE: won't adopt if this stream wraps another stream.
	Q_INVOKABLE Stream* adopt(const QVariant& target);

	bool isWrapper() const;
	bool isAbortable() const;
	bool isAborted() const;
};

} // quickstreams

Q_DECLARE_METATYPE(quickstreams::StreamHandle*)
