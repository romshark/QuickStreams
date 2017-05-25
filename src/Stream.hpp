#pragma once

#include "StreamHandle.hpp"
#include <QObject>
#include <QJSValue>
#include <QQmlEngine>
#include <QVariant>
#include <QString>
#include <QMetaType>
#include <QSet>

namespace quickstreams {

class Streams;

class Stream : public QObject {
	Q_OBJECT
	friend class StreamProvider;

public:
	enum class State : char {Dead, Active, Aborted};
	Q_ENUM(State)

	enum class WakeCondition : char {Default, Abort};
	Q_ENUM(WakeCondition)

	enum class Type : char {Atomic, Abortable};
	Q_ENUM(Type)

	enum class Belonging : char {Free, Attached, Bound, Wrapped};
	Q_ENUM(Belonging)

	enum class NextType : char {Attached, Bound, None};
	Q_ENUM(NextType)

protected:
	QQmlEngine* _engine;
	Type _type;
	State _state;
	Belonging _belonging;
	QJSValue _function;
	Stream* _parent;
	NextType _nextType;
	QVariantList _retryErrSamples;
	qint32 _maxTrials;
	qint32 _currentTrial;
	QJSValue _repeatCondition;
	QSet<QString> _observedEvents;
	StreamHandle _handle;

	explicit Stream(
		QQmlEngine* engine,
		const QJSValue& function,
		Type type = Type::Atomic,
		Belonging belonging = Belonging::Free,
		QObject* parent = nullptr
	);
	void emitEvent(const QString& name, const QVariant& data);
	void emitClosed(const QVariant& data);
	void emitFailed(const QVariant& reason, WakeCondition wakeCondition);
	bool verifyErrorListed(const QVariant& err) const;
	Stream* subsequentStream(
		const QJSValue& target,
		Type streamType,
		Belonging belonging
	);
	void setParentStream(Stream* parentStream);
	void connectSubsequent(Stream* stream);

protected slots:
	// Initialize is called on stream creation to delay execution.
	void initialize();

	// Registers a stream to awake if this stream fails.
	// Propagate to superordinate streams
	void registerFailureRecoveryStream(Stream* failureStream);

	// Registers a stream to awake if this stream is aborted.
	// Propagate to superordinate streams
	void registerAbortionRecoveryStream(Stream* abortionStream);

	// Awakes this stream when a superordinate stream is closed
	void awake(
		QVariant data = QVariant(),
		quickstreams::Stream::WakeCondition wakeCondition =
			quickstreams::Stream::WakeCondition::Default
	);

	// Awakes this stream when a superordinate stream is closed
	void awakeFromEvent(QString name, QVariant data = QVariant());

	void handleParentAbortion();

signals:
	void eventEmitted(QString name, QVariant data);
	void closed(QVariant data, WakeCondition wakeCondition);
	void failed(QVariant error, WakeCondition wakeCondition);
	void aborted(QVariant reason, WakeCondition wakeCondition);
	void abortChildren();
	void propagateFailureStream(Stream* failureStream);
	void propagateAbortionStream(Stream* abortionStream);

public:
	// retry is a stream operator, it repeats resurrecting the current stream
	// if either of the given error samples match the catched error.
	Q_INVOKABLE Stream* retry(
		const QVariant& samples,
		const QJSValue& maxTrials = QJSValue()
	);

	// repeat is a stream operator, it repeats resurrecting the current stream
	// if the given condition returns true.
	Q_INVOKABLE Stream* repeat(const QJSValue& condition);

	// attach is a stream operator, it creates a new stream that is awoken
	// when the current stream is successfuly closed.
	//
	// NOTICE: the next stream will inherit the superordinate streams parent.
	Q_INVOKABLE Stream* attach(const QJSValue& target);

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
	Q_INVOKABLE Stream* bind(const QJSValue& target);

	//TODO: implement
	// event is a stream operator, it creates and returns a new stream
	// that is awoken when a certain set of events occures.
	Q_INVOKABLE Stream* event(const QVariant& name, QJSValue callback);

	// failure is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain fails.
	Q_INVOKABLE Stream* failure(const QJSValue& target);

	// abortion is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain is aborted.
	Q_INVOKABLE Stream* abortion(const QJSValue& target);

	// abort is a stream method that allows to abort this stream.
	// This method does nothing if this stream is atomic.
	Q_INVOKABLE void abort();

	// isAbortable returns false if this stream is atomic, otherwise true.
	Q_INVOKABLE bool isAbortable() const;

	// isAborted returns true if this stream was aborted, otherwise false.
	Q_INVOKABLE bool isAborted() const;
};

} // quickstreams

Q_DECLARE_METATYPE(quickstreams::Stream*)
Q_DECLARE_METATYPE(quickstreams::Stream::Type)
