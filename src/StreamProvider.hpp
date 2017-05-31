#pragma once

#include "Stream.hpp"
#include <QObject>
#include <QQmlEngine>
#include <QJSValue>

namespace quickstreams {

class StreamProvider : public QObject {
	Q_OBJECT
	Q_PROPERTY(quickstreams::Stream::Type Atomic READ Atomic CONSTANT)
	Q_PROPERTY(quickstreams::Stream::Type Abortable READ Abortable CONSTANT)

protected:
	QQmlEngine* _engine;

public:
	explicit StreamProvider(QQmlEngine* engine, QObject* parent = nullptr);
	Q_INVOKABLE Stream* create(
		const QJSValue& target,
		quickstreams::Stream::Type type = Stream::Type::Atomic
	);

	quickstreams::Stream::Type Atomic() const;
	quickstreams::Stream::Type Abortable() const;

	//TODO: implement
	// Join returns a new stream that's closed when all given sub-streams
	// are closed. If either of them fails the returned stream will fail.
	// If either of them is canceled the returned stream will fail as well	.
	/*
	Q_INVOKABLE Stream* join(...);
	*/
};

} // quickstreams
