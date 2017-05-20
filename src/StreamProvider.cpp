#include "StreamProvider.hpp"
#include "Stream.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
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
	const QJSValue& target
) {
	auto stream(new Stream(_engine, target));
	QMetaObject::invokeMethod(stream, "awake", Qt::QueuedConnection);
	return stream;
}
