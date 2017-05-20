#include "StreamProvider.hpp"
#include "Stream.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
#include <QMetaObject>

streams::StreamProvider::StreamProvider(
	QQmlEngine* engine,
	QObject *parent
) :
	QObject(parent),
	_engine(engine)
{

}

streams::Stream* streams::StreamProvider::create(const QJSValue& target) {
	auto stream(new Stream(_engine, target));
	QMetaObject::invokeMethod(stream, "awake", Qt::QueuedConnection);
	return stream;
}
