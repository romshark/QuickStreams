#pragma once

#include "ProviderInterface.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "LambdaExecutable.hpp"
#include "Repeater.hpp"
#include "LambdaRepeater.hpp"
#include "Retryer.hpp"
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
		New, Canceled,
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

	enum class ControlFlowBranching : char {
		None,
		Failure,
		Abortion,
		Both
	};
	Q_ENUM(ControlFlowBranching)

protected:
	ProviderInterface* _provider;
	Type _type;
	State _state;
	ControlFlowBranching _conFlowBranching;
	Captured _captured;
	CaptionStatus _captionStatus;
	Stream* _parent;
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
	Reference create(
		const Executable::Reference& executable,
		Type type,
		CaptionStatus captionStatus
	) const;

	// Return true if the attach attempt was faulty
	// for any of the following reasons:
	// 1. this stream already captured another stream (no sequence splitting)
	// 2. attempted to attach a non-free stream
	// 3. attempted to attach a stream to itself
	bool cannotAttach() const;
	bool cannotAttachStream(const Reference& stream) const;
	bool cannotBind() const;
	bool cannotBindStream(const Reference& stream) const;

	// Return true if this stream already registered a failure branch
	// to prevent multiple failure branches and invokations
	// of a stream from multiple different sequences

	// Return true if the attempt to register a failure sequence was faulty
	// for any of the following reasons:
	// 1. this stream already registered another failure sequence
	// 2. attempted to register a non-free stream
	// 3. attempted to register a stream to itself
	bool cannotBranchFailure() const;
	bool cannotBranchFailureStream(const Reference& stream) const;

	// Return true if the attempt to register an abortion sequence was faulty
	// for any of the following reasons:
	// 1. this stream already registered another abortion sequence
	// 2. attempted to register a non-free stream
	// 3. attempted to register a stream to itself
	bool cannotBranchAbortion() const;
	bool cannotBranchAbortionStream(const Reference& stream) const;

	// Throws a warning if the initial stream of a failure sequence
	// is a member of the sequence to branch off in case of failure
	void verifyFailureSequenceStream(Stream* stream) const;

	// Throws a warning if the initial stream of an abortion sequence
	// is a member of the sequence to branch off in case of abortion
	void verifyAbortionSequenceStream(Stream* stream) const;

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

	// Registers the first stream of the sequence to awake
	// when this stream fails.
	// Recursively propagate it to the entire sequence
	void registerFailureSequence(Stream* failureStream);

	// Registers the first stream of the sequence to awake
	// when this stream is closed after it's aborted.
	// Recursively propagate it to the entire sequence
	void registerAbortionSequence(Stream* abortionStream);

	// Awakes this stream, when the preceding stream either closes
	// or redirects control flow to this stream after a failure or an abortion
	void awake(
		QVariant data = QVariant(),
		quickstreams::Stream::WakeCondition wakeCondition =
			quickstreams::Stream::WakeCondition::Default
	);

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
	void propagateFailureStream(Stream* failureStream);
	void propagateAbortionStream(Stream* abortionStream);

	// Eliminates all subordinate streams
	void eliminateSubordinate();

	// Eliminates the current sequence recursively
	void eliminateSequence();

	// Eliminates the declared failure sequence recursively
	void eliminateFailureSequence();

	// Eliminates the declared abortion sequence recursively
	void eliminateAbortionSequence();

public:
	// delay is a stream operator, it delays the awakening of the stream
	// for the given amount of milliseconds. If the stream is abortable
	// and aborted during the delay - the delay timer is stopped,
	// the stream is canceled and never awoken. But if it's an atomic stream
	// the delay will block abortion until the stream is finally awoken.
	Reference delay(int duration);

	// retry is a stream operator, it repeats resurrecting the current stream
	// if either of the given error samples match the catched error.
	Reference retry(const QVariantList& samples, qint32 maxTrials = -1);

	// repeat is a stream operator, it repeats resurrecting the current stream
	// if the given condition returns true.
	Reference repeat(Repeater::Reference newRepeater);
	Reference repeat(LambdaRepeater::Function function);


	// attach is a stream operator, it creates a new stream that is awoken
	// when the current stream is successfuly closed.
	//
	// NOTICE: the next stream will inherit the superordinate streams parent.
	Reference attach(const Executable::Reference& executable);
	Reference attach(LambdaExecutable::Function prototype);
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
	Reference bind(LambdaExecutable::Function prototype);
	Reference bind(const Reference& stream);

	// event is a stream operator, it creates and returns a new stream
	// that is awoken when a certain set of events occures.
	Reference event(const QString& name, const Callback::Reference& callback);

	// failure is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain fails.
	Reference failure(const Executable::Reference& executable);
	Reference failure(LambdaExecutable::Function prototype);
	Reference failure(const Reference& stream);

	// abortion is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain is aborted.
	Reference abortion(const Executable::Reference& executable);
	Reference abortion(LambdaExecutable::Function prototype);
	Reference abortion(const Reference& stream);

	// abort is a stream method that allows to abort this stream.
	// This method does nothing if this stream is atomic.
	void abort();

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
