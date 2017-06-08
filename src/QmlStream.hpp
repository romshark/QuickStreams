#pragma once

#include "Stream.hpp"
#include "QmlStreamHandle.hpp"
#include "Executable.hpp"
#include "Provider.hpp"
#include <QObject>
#include <QMetaType>
#include <QSharedPointer>
#include <QTimer>
#include <QString>
#include <QSet>
#include <QQmlEngine>
#include <QJSValue>
#include <QVariant>

namespace quickstreams {
namespace qml {

class QmlProvider;

class QmlStream : public QObject {
	Q_OBJECT
	friend class quickstreams::qml::QmlProvider;

protected:
	QQmlEngine* _engine;
	quickstreams::Stream::Reference _reference;
	QmlStreamHandle _handle;

	explicit QmlStream(
		QQmlEngine* engine,
		quickstreams::Stream::Reference reference
	);
	QmlStream* fromJsValue(
		const QJSValue& value,
		quickstreams::Stream::Type type,
		quickstreams::Stream::Capture capture
	);

public:
	// delay is a stream operator, it delays the awakening of the stream
	// for the given amount of milliseconds. If the stream is abortable
	// and aborted during the delay - the delay timer is stopped,
	// the stream is canceled and never awoken. But if it's an atomic stream
	// the delay will block abortion until the stream is finally awoken.
	Q_INVOKABLE QmlStream* delay(const QJSValue& duration);

	// retry is a stream operator, it repeats resurrecting the current stream
	// if either of the given error samples match the catched error.
	Q_INVOKABLE QmlStream* retry(
		const QVariant& samples,
		const QJSValue& maxTrials = QJSValue()
	);

	// repeat is a stream operator, it repeats resurrecting the current stream
	// if the given condition returns true.
	Q_INVOKABLE QmlStream* repeat(const QJSValue& condition);

	// attach is a stream operator, it creates a new stream that is awoken
	// when the current stream is successfuly closed.
	//
	// NOTICE: the next stream will inherit the superordinate streams parent.
	Q_INVOKABLE QmlStream* attach(const QJSValue& target);

	// bind is a stream operator, it creates a new appended stream
	// that is guaranteed to be awoken when the current stream
	// is successfuly closed. Unlike a stream created by the next operator
	// it will be awoken even if the chain is aborted.
	//
	// NOTICE: The appended stream will inherit
	// the superordinate streams parent.
	//
	// NOTICE: Streams appended to abortable streams won't be awoken
	// if the abortable stream was aborted.
	Q_INVOKABLE QmlStream* bind(const QJSValue& target);

	//TODO: implement
	// event is a stream operator, it creates and returns a new stream
	// that is awoken when a certain set of events occures.
	Q_INVOKABLE QmlStream* event(
		const QVariant& name,
		const QJSValue& callback
	);

	// failure is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain fails.
	Q_INVOKABLE QmlStream* failure(const QJSValue& target);

	// abortion is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain is aborted.
	Q_INVOKABLE QmlStream* abortion(const QJSValue& target);

	// abort is a stream method that allows to abort this stream.
	// This method does nothing if this stream is atomic.
	Q_INVOKABLE void abort();

	// isAbortable returns false if this stream is atomic, otherwise true.
	Q_INVOKABLE bool isAbortable() const;

	// isAborted returns true if this stream was aborted, otherwise false.
	Q_INVOKABLE bool isAborted() const;
};

}} // quickstreams::qml

Q_DECLARE_METATYPE(quickstreams::qml::QmlStream*)
