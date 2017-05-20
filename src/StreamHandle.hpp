#pragma once

#include <functional>
#include <QObject>
#include <QString>
#include <QVariant>

namespace streams {

class Stream;

class StreamHandle {
	Q_GADGET

public:
	typedef std::function<void(const QString&, const QVariant&)> EventCallback;
	typedef std::function<void(const QVariant&)> CloseCallback;
	typedef std::function<void(const QVariant&)> FailCallback;
	typedef std::function<bool(const QVariant&)> AwaitCallback;

protected:
	bool _awaiting;
	EventCallback _eventCb;
	CloseCallback _closeCb;
	FailCallback _failCb;
	AwaitCallback _awaitCb;

public:
	StreamHandle();
	StreamHandle(
		EventCallback eventCb,
		CloseCallback closeCb,
		FailCallback failCb,
		AwaitCallback awaitCb
	);

	// event makes the stream immediately emit an event
	// optionally passing any data. It doesn't matter if the stream is
	// awaiting another stream, an event will fire in either case.
	Q_INVOKABLE void event(
		const QString& name,
		const QVariant& data = QVariant()
	);

	// close immediately closes the stream optionally passing any data.
	//
	// NOTICE: close won't trigger if the stream is awaiting another stream.
	Q_INVOKABLE void close(const QVariant& data = QVariant());

	// fail immediately fails the stream optionally passing any data
	// describing the reason of failure.
	//
	// NOTICE: fail won't trigger if the stream is awaiting another stream.
	Q_INVOKABLE void fail(const QVariant& data = QVariant());

	// await takes a stream and attaches to its close and failure signals.
	Q_INVOKABLE void await(const QVariant& data);

	Q_INVOKABLE bool isAwaiting() const;
};

} // streams
