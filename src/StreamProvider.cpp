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

quickstreams::Stream::Type quickstreams::StreamProvider::Atomic() const {
	return Stream::Type::Atomic;
}
quickstreams::Stream::Type quickstreams::StreamProvider::Abortable() const {
	return Stream::Type::Abortable;
}
