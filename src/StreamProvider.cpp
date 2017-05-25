#include "StreamProvider.hpp"
#include "Stream.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
#include <QtQml>
#include <QMetaObject>

quickstreams::StreamProvider::StreamProvider(
	QQmlEngine* engine,
	QObject *parent
) :
	QObject(parent),
	_engine(engine)
{

}

quickstreams::Stream* quickstreams::StreamProvider::create(
	const QJSValue& target,
	quickstreams::Stream::Type type
) {
	auto stream(new Stream(_engine, target, type, Stream::Belonging::Free));
	QMetaObject::invokeMethod(stream, "initialize", Qt::QueuedConnection);
	return stream;
}

quickstreams::Stream* quickstreams::StreamProvider::timeout(
	const QJSValue& delay,
	const QJSValue& target
) {
	Stream* stream;
	if(target.toVariant().canConvert<Stream*>()) {
		stream = qjsvalue_cast<Stream*>(target);
		stream->_belonging = Stream::Belonging::Bound;
	} else {
		// It's safe to pass a non-function target, If it's not a function
		// the stream will simply ignore it without executing
		stream = new Stream(
			_engine, target, Stream::Type::Atomic, Stream::Belonging::Bound
		);
	}
	if(delay.isNumber()) {
		QTimer::singleShot(delay.toUInt(), [stream]() {
			stream->awake();
		});
	} else {
		QTimer::singleShot(0, [stream]() {
			stream->awake();
		});
	}
	return stream;
}

quickstreams::Stream::Type quickstreams::StreamProvider::Atomic() const {
	return Stream::Type::Atomic;
}
quickstreams::Stream::Type quickstreams::StreamProvider::Abortable() const {
	return Stream::Type::Abortable;
}
