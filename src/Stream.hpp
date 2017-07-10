#pragma once

#include "ProviderInterface.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "LambdaExecutable.hpp"
#include "LambdaSyncExecutable.hpp"
#include "LambdaWrapper.hpp"
#include "Repeater.hpp"
#include "LambdaRepeater.hpp"
#include "Retryer.hpp"
#include "TypeRetryer.hpp"
#include "LambdaRetryer.hpp"
#include "Callback.hpp"
#include <QObject>
#include <QJSValue>
#include <QVariant>
#include <QVariantList>
#include <QString>
#include <QMetaType>
#include <QMultiHash>
#include <QTimer>
#include <QSharedPointer>

namespace quickstreams {

class Provider;

namespace qml {

class QmlStream;
class QmlProvider;

}

class Stream : public QObject {
	Q_OBJECT
	friend class quickstreams::Provider;
	friend class quickstreams::qml::QmlStream;
	friend class quickstreams::qml::QmlProvider;

public:
	typedef QSharedPointer<quickstreams::Stream> Reference;

	enum class State : char {
		Initializing,
		Awaiting,
		Canceled,
		Active, AwaitingDelay, Aborted,
		Dead
	};
	Q_ENUM(State)

	enum class WakeCondition : char {
		Default,
		Abort,
		DefaultNoDelay,
		AbortNoDelay,
	};
	Q_ENUM(WakeCondition)

	enum class Type : char {Atomic, Abortable};
	Q_ENUM(Type)

	enum class Captured : char {None, Attached, Bound};
	Q_ENUM(Captured)

	enum class CaptionStatus : char {Free, Attached, Bound};
	Q_ENUM(CaptionStatus)

	static Executable::Reference Wrap(LambdaWrapper::Function function);

protected:
	ProviderInterface* _provider;
	Type _type;
	State _state;
	Captured _captured;
	CaptionStatus _captionStatus;
	Stream* _parent;
	Stream* _failure;
	Stream* _abortion;
	QMultiHash<QString, Callback::Reference> _observedEvents;
	StreamHandle _handle;

	// Optional members and operators
	Executable::Reference _executable;
	QTimer* _awakeningTimer;
	Retryer::Reference _retryer;
	Repeater::Reference _repeater;

	explicit Stream(
		ProviderInterface* provider,
		const Executable::Reference& executable,
		Type type = Type::Atomic,
		CaptionStatus captionStatus = CaptionStatus::Free
	);
	~Stream();

	Reference create(
		const Executable::Reference& executable,
		Type type,
		CaptionStatus captionStatus
	) const;

	// Throws an exception if the attach attempt was faulty
	// for any of the following reasons:
	// 1. this stream already captured another stream (no sequence splitting)
	// 2. attempted to attach a non-free stream
	// 3. attempted to attach a stream to itself
	void verifyAttach() const;
	void verifyAttachStream(const Reference& stream) const;
	void verifyBind() const;
	void verifyBindStream(const Reference& stream) const;

	// Throws an exception if the attempt to register a failure sequence
	// was faulty for any of the following reasons:
	// 1. this stream already registered another failure sequence
	// 2. attempted to register a non-free stream
	// 3. attempted to register a stream to itself
	void verifyFailureSequence() const;
	void verifyFailureSequenceStream(Stream* stream) const;

	// Throws an exception if the attempt to register an abortion sequence
	// was faulty for any of the following reasons:
	// 1. this stream already registered another abortion sequence
	// 2. attempted to register a non-free stream
	// 3. attempted to register a stream to itself
	void verifyAbortionSequence() const;
	void verifyAbortionSequenceStream(Stream* stream) const;

	// Throws an exception if the initial stream of a failure sequence
	// is a member of the sequence to branch off in case of failure
	void verifyFailSeqStreamNotMember(Stream* stream) const;

	// Throws an exception if the initial stream of an abortion sequence
	// is a member of the sequence to branch off in case of abortion
	void verifyAbortSeqStreamNotMember(Stream* stream) const;

	// Registers the first stream of the sequence to awake
	// when this stream fails.
	// Recursively propagate it to the entire sequence
	void registerFailureSequence(Stream* failureStream);
	void connectFailureSequence(Stream* failureStream);

	// Registers the first stream of the sequence to awake
	// when this stream is closed after it's aborted.
	// Recursively propagate it to the entire sequence
	void registerAbortionSequence(Stream* abortionStream);
	void connectAbortionSequence(Stream* abortionStream);

	Reference adopt(Reference another);
	void emitEvent(const QString& name, const QVariant& data) const;
	void emitClosed(const QVariant& data);
	void emitFailed(const QVariant& reason, WakeCondition wakeCondition);
	void setSuperordinateStream(Stream* stream);
	void connectSubsequent(Stream* stream);
	void die();

protected slots:
	// The stream is asynchronously initialized
	// after it's creation by the provider.
	// If it remained uncaptured - thus free,
	// it will be awoken right away
	void initialize();

	void onPropagateFailSeqUp(Stream* initialStream);
	void onPropagateFailSeqDown(Stream* initialStream);

	void onPropagateAbortSeqUp(Stream* initialStream);
	void onPropagateAbortSeqDown(Stream* initialStream);

	// Awakes this stream, when the preceding stream either closes
	// or redirects control flow to this stream after a failure or an abortion
	void awake(
		QVariant data = QVariant(),
		quickstreams::Stream::WakeCondition wakeCondition =
			quickstreams::Stream::WakeCondition::Default
	);

	// Handles sequence initialization signals,
	// must transit this stream into Awaiting state
	// and reemit initializeSequences signal to subsequent streams
	void onInitialize();

	// Handles sequence eliminatation signal, must cancel this stream
	// and recursively eliminate all subsequent streams
	void onEliminateSequence();

signals:
	void eventEmitted(QString name, QVariant data);
	void closed(QVariant data, WakeCondition wakeCondition);
	void failed(QVariant error, WakeCondition wakeCondition);
	void aborted(QVariant reason, WakeCondition wakeCondition);
	void retryIteration(QVariant data, WakeCondition wakeCondition);
	void repeatIteration(QVariant data, WakeCondition wakeCondition);
	void abortSubordinate();

	void propagateFailSeqUp(Stream* initialStream);
	void propagateFailSeqDown(Stream* initialStream);

	void propagateAbortSeqUp(Stream* initialStream);
	void propagateAbortSeqDown(Stream* initialStream);

	// Eliminates all subordinate streams
	void eliminateSubordinate();

	// Eliminates the current sequence recursively
	void eliminateSequence();

	// Eliminates the declared failure sequence recursively
	void eliminateFailureSequence();

	// Eliminates the declared abortion sequence recursively
	void eliminateAbortionSequence();

	// The Initialization signal is fired when the initial free stream
	// is scheduled for execution. It makes subsequent streams transit into
	// the awaiting state to prevent control flow manipulation at runtime
	void initializeSequences();

public:
	// delay is a stream operator, it delays the awakening of the stream
	// for the given amount of milliseconds. If the stream is abortable
	// and aborted during the delay - the delay timer is stopped,
	// the stream is canceled and never awoken. But if it's an atomic stream
	// the delay will block abortion until the stream is finally awoken.
	Reference delay(int duration);

	// retry is a stream operator, it repeats resurrecting the current stream
	// if either of the given error samples match the catched error.
	Reference retry(Retryer::Reference newRetryer);
	Reference retry(
		const TypeRetryer::TypeList& errorTypes,
		qint32 maxTrials = -1
	);
	Reference retry(
		LambdaRetryer::Function function,
		qint32 maxTrials = -1
	);

	// repeat is a stream operator, it repeats resurrecting the current stream
	// if the given condition returns true.
	Reference repeat(Repeater::Reference newRepeater);
	Reference repeat(LambdaRepeater::Function function);


	// attach is a stream operator, it creates a new stream that is awoken
	// when the current stream is successfuly closed.
	//
	// NOTICE: the next stream will inherit the superordinate streams parent.
	Reference attach(const Executable::Reference& executable);
	Reference attach(LambdaSyncExecutable::Function function);
	Reference attach(const Reference& stream);

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
	Reference bind(const Executable::Reference& executable);
	Reference bind(LambdaSyncExecutable::Function function);
	Reference bind(const Reference& stream);

	// event is a stream operator, it creates and returns a new stream
	// that is awoken when a certain set of events occures.
	Reference event(const QString& name, const Callback::Reference& callback);

	// failure is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain fails.
	Reference failure(const Executable::Reference& executable);
	Reference failure(LambdaSyncExecutable::Function function);
	Reference failure(const Reference& stream);

	// abortion is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain is aborted.
	Reference abortion(const Executable::Reference& executable);
	Reference abortion(LambdaSyncExecutable::Function function);
	Reference abortion(const Reference& stream);

	// abort is a stream method that allows to abort this stream.
	// This method does nothing if this stream is atomic.
	void abort();

	// Returns the state the stream is currently in
	State state() const;

	// Returns false if this stream is atomic, otherwise returns true.
	bool isAbortable() const;

	// Returns true if this stream was aborted, otherwise returns false.
	bool isAborted() const;

	// Returns true if this stream is either new, canceled or dead,
	// otherwise returns false
	bool isInactive() const;
};

} // quickstreams

Q_DECLARE_METATYPE(quickstreams::Stream::Type)
Q_DECLARE_METATYPE(quickstreams::Stream::State)
