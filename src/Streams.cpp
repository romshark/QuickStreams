#include "Streams.hpp"
#include "Stream.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>
#include <QMetaObject>

streams::Streams::Streams(QQmlEngine* engine, QObject *parent) :
	QObject(parent),
	_engine(engine)
{

}

streams::Stream* streams::Streams::create(const QJSValue& target) {
	auto stream(new Stream(_engine, target));
	QMetaObject::invokeMethod(stream, "awake", Qt::QueuedConnection);
	return stream;
}
