#include "Stream.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "LambdaExecutable.hpp"
#include "LambdaSyncExecutable.hpp"
#include "LambdaWrapper.hpp"
#include "Repeater.hpp"
#include "LambdaRepeater.hpp"
#include <exception>
#include <QJSValue>
#include <QList>
#include <QString>
#include <QVariant>
#include <QMetaObject>
#include <QTimer>
#include <QSharedPointer>
#include <QDebug>

quickstreams::Executable::Reference quickstreams::Stream::Wrap(
	quickstreams::LambdaWrapper::Function function
) {
	return Executable::Reference(new LambdaWrapper(function));
}

quickstreams::Stream::Stream(
	ProviderInterface* provider,
	const Executable::Reference& executable,
	Type type,
	CaptionStatus captionStatus
) :
	QObject(nullptr),
	_provider(provider),
	_type(type),
	_state(State::Initializing),
	_captured(Captured::None),
	_captionStatus(captionStatus),
	_parent(nullptr),
	_handle(
		// Called when stream is requested to emit an event
		[this](const QString& name, const QVariant& data) {
			emitEvent(name, data);
		},
		// Called when stream is requested to close
		[this](const QVariant& data) {
			emitClosed(data);
		},
		// Called when stream is requested to fail
		[this](const QVariant& reason) {
			emitFailed(reason, WakeCondition::Default);
		},
		// Called when stream is requested to adopt another stream
		[this](Reference& reference) {
			return adopt(reference);
		},
		// Called when isAbortable is requested
		[this]() {
			return isAbortable();
		},
		// Called when isAborted is requested
		[this]() {
			return isAborted();
		}
	),
	_executable(executable),
	_awakeningTimer(nullptr),
	_retryer(nullptr),
	_repeater(nullptr)
{
	if(!_executable.isNull()) _executable->setHandle(&_handle);
	connect(
		this, &Stream::retryIteration,
		this, &Stream::awake,
		Qt::QueuedConnection
	);
	connect(
		this, &Stream::repeatIteration,
		this, &Stream::awake,
		Qt::QueuedConnection
	);
}

quickstreams::Stream::~Stream() {
	_provider->destroyed();
}

quickstreams::Stream::Reference quickstreams::Stream::create(
	const Executable::Reference& executable,
	Type type,
	CaptionStatus captionStatus
) const {
	Stream::Reference reference(new Stream(
		_provider, executable, type, captionStatus
	), &Stream::deleteLater);
	_provider->registerNew(reference);
	return reference;
}

void quickstreams::Stream::verifyAttach() const {
	// Attach operator cannot be used past the initialization phase
	if(_state != State::Initializing) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to use the attach operator during the runtime!"
	);

	// A stream cannot attach-capture multiple other streams
	if(_captured != Captured::None) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to attach a stream (created from a prototype) "
		"to a stream that already captures another one!"
	);
}

void quickstreams::Stream::verifyAttachStream(const Reference& stream) const {
	verifyAttach();

	// A stream cannot attach-capture itself
	if(stream.data() == this) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to attach a stream to itself!"
	);

	// A stream cannot attach-capture a non-free stream
	if(stream->_captionStatus != CaptionStatus::Free) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to attach a non-free stream!"
	);
}

void quickstreams::Stream::verifyBind() const {
	// Bind operator cannot be used past the initialization phase
	if(_state != State::Initializing) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to use the bind operator during the runtime!"
	);

	//A stream cannot bind-capture multiple other streams
	if(_captured != Captured::None) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to bind a stream (created from a prototype) "
		"to a stream that already captures another one!"
	);
}

void quickstreams::Stream::verifyBindStream(const Reference& stream) const {
	verifyBind();

	// A stream cannot bind-capture itself
	if(stream.data() == this) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to bind a stream to itself!"
	);

	// A stream cannot bind-capture a non-free stream
	if(stream->_captionStatus != CaptionStatus::Free) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to bind a non-free stream!"
	);
}

void quickstreams::Stream::verifyFailureSequence() const {
	// Failure operator cannot be used past the initialization phase
	if(_state != State::Initializing) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to use the failure operator during the runtime!"
	);
}

void quickstreams::Stream::verifyFailureSequenceStream(Stream* stream) const {
	verifyFailureSequence();

	// A stream cannot register itself
	if(stream == this) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to register a stream to itself "
		"as the initial stream of a failure sequence!"
	);

	// A stream cannot be captured by multiple individual streams
	if(stream->_captionStatus != CaptionStatus::Free) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to register a non-free stream "
		"as the initial stream of a failure sequence!"
	);
}

void quickstreams::Stream::verifyAbortionSequence() const {
	// Abortion operator cannot be used past the initialization phase
	if(_state != State::Initializing) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to use the abortion operator during the runtime!"
	);
}

void quickstreams::Stream::verifyAbortionSequenceStream(Stream* stream) const {
	verifyAbortionSequence();

	// A stream cannot register itself
	if(stream == this) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to register a stream to itself "
		"as the initial stream of an abortion sequence!"
	);

	// A stream cannot be captured by multiple individual streams
	if(stream->_captionStatus != CaptionStatus::Free) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Attempted to register a non-free stream "
		"as the initial stream of an abortion sequence!"
	);
}

void quickstreams::Stream::verifyFailSeqStreamNotMember(
	Stream* stream
) const {
	if(this == stream) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Registered a sequence member stream as the initial stream "
		"of a failure recovery sequence branching off it!"
	);
}

void quickstreams::Stream::verifyAbortSeqStreamNotMember(
	Stream* stream
) const {
	if(this == stream) throw std::logic_error(
		"QuickStreams - FATAL ERROR: "
		"Registered a sequence member stream as the initial stream "
		"of an abortion recovery sequence branching off it!"
	);
}

quickstreams::Stream::Reference quickstreams::Stream::adopt(
	Reference another
) {
	if(another.isNull()) {
		auto reference(create(nullptr, Type::Atomic, CaptionStatus::Bound));
		another.swap(reference);
	}
	// Acquire ownership over the adopted stream
	another->setSuperordinateStream(this);
	return another;
}

void quickstreams::Stream::emitEvent(
	const QString& name,
	const QVariant& data
) const {
	if(!_observedEvents.contains(name)) return;

	// Execute all registered callbacks
	QList<Callback::Reference> callbacks(_observedEvents.values(name));
	for(
		QList<Callback::Reference>::const_iterator itr(
			callbacks.constBegin()
		);
		itr != callbacks.constEnd();
		itr++
	) {
		if(itr->isNull()) continue;
		(*itr)->execute(data);
	}
}

void quickstreams::Stream::emitClosed(const QVariant& data) {
	// Dead and canceled streams can't be closed
	if(isInactive()) return;

	// Reset trial counter on success
	if(!_retryer.isNull()) _retryer->reset();

	// Check whether repeat is desired
	if(!_repeater.isNull()) {
		if(_repeater->evaluate(isAborted())) {
			// Repeat asynchronously resurrecting this stream in another tick
			repeatIteration(data, WakeCondition::Default);
			return;
		}
	}

	// If there is no next stream but this stream was aborted
	// the abortion recovery stream should be invoked next.
	// Otherwise execute the subsequent bound stream
	// with the condition that its state is initially 'Aborted'
	if(isAborted()) {
		switch(_captured) {
		case Captured::Bound:
			closed(data, WakeCondition::Abort);
			// Die but don't touch any other sequence,
			// forward cleanup responsibility to the bound stream.
			die();
			return;
		default:
			aborted(data, WakeCondition::Default);
			// Die and cancel unreachable sequences
			// (current sequence and the failure sequence)
			die();
			eliminateSequence();
			eliminateFailureSequence();
			return;
		}
	}

	// Otherwise close this stream initializing the next stream
	closed(data, WakeCondition::Default);
	die();

	// If this stream represents the end of a sequence
	// then eliminate the unreachable failure and abortion sequences
	if(_captured == Captured::None) {
		eliminateFailureSequence();
		eliminateAbortionSequence();
	}
}

void quickstreams::Stream::emitFailed(
	const QVariant& data,
	WakeCondition wakeCondition
) {
	// Dead and canceled stream can't fail
	if(isInactive()) return;

	// Check whether retrial is desired
	if(_retryer != nullptr) {
		_retryer->incrementTrialCounter();
		if(_retryer->verify(data)) {
			// Retry asynchronously
			retryIteration(data, wakeCondition);
			return;
		}
	}

	// Otherwise fail this stream and redirect control flow
	// to the failure recovery sequence
	failed(data, WakeCondition::Default);
	// Die and cancel unreachable sequences
	// (current sequence and the abortion sequence)
	die();
	eliminateSequence();
	eliminateAbortionSequence();
}

void quickstreams::Stream::setSuperordinateStream(Stream *stream) {
	// Remember parent stream for automatic inheritance
	_parent = stream;

	// Immediately react to parents abortion
	connect(
		_parent, &Stream::abortSubordinate,
		this, &Stream::abort,
		Qt::DirectConnection
	);

	// Immediately die on parents elimination command
	connect(
		_parent, &Stream::eliminateSubordinate,
		this, &Stream::die,
		Qt::DirectConnection
	);
}

void quickstreams::Stream::connectSubsequent(Stream* stream) {
	// Receive failure and abortion sequence propagation signals
	// during the declaration in both directions (up and down)
	connect(
		this, &Stream::propagateFailSeqUp,
		stream, &Stream::onPropagateFailSeqUp,
		Qt::DirectConnection
	);
	connect(
		stream, &Stream::propagateFailSeqDown,
		this, &Stream::onPropagateFailSeqDown,
		Qt::DirectConnection
	);

	connect(
		this, &Stream::propagateAbortSeqUp,
		stream, &Stream::onPropagateAbortSeqUp,
		Qt::DirectConnection
	);
	connect(
		stream, &Stream::propagateAbortSeqDown,
		this, &Stream::onPropagateAbortSeqDown,
		Qt::DirectConnection
	);

	// Initialize the stream and the following sequence recursively
	connect(
		this, &Stream::initializeSequences,
		stream, &Stream::onInitialize,
		Qt::DirectConnection
	);

	// Immediately kill the subsequent stream including the entire sequence
	connect(
		this, &Stream::eliminateSequence,
		stream, &Stream::onEliminateSequence,
		Qt::DirectConnection
	);

	// When this stream closes - awake the next stream
	connect(
		this, &Stream::closed,
		stream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Automatically inherit parent stream
	if(_parent) stream->setSuperordinateStream(_parent);
}

void quickstreams::Stream::die() {
	// Initializing and awaiting streams become canceled,
	// Active, Aborted and streams awaiting their delay become Dead
	switch(_state) {
	case State::Dead:
		return;
	case State::Initializing:
	case State::Awaiting:
		_state = State::Canceled;
		break;
	case State::Active:
		_provider->finished();
	default:
		_state = State::Dead;
		break;
	}

	_provider->dispose(this);

	// Eliminate all subordinate streams
	eliminateSubordinate();
}

void quickstreams::Stream::initialize() {
	if(_captionStatus != CaptionStatus::Free) return;

	// Transit all subsequent streams as well as the abortion
	// and failure streams into the Awaiting state locking them
	// to prevent further manipulation through operators at runtime
	initializeSequences();
	_state = State::Awaiting;

	awake(QVariant(), WakeCondition::Default);
}

void quickstreams::Stream::registerFailureSequence(Stream* initialStream) {
	verifyFailSeqStreamNotMember(initialStream);

	// Disconnect and eliminate (override) the registered failure sequence
	disconnect(this, &Stream::failed, 0, 0);
	eliminateFailureSequence();

	// Initialize the failure sequence and the following sequence recursively
	connect(
		this, &Stream::initializeSequences,
		initialStream, &Stream::onInitialize,
		Qt::DirectConnection
	);

	// Asynchronously awake failure recovery sequence
	// if this stream fails
	connect(
		this, &Stream::failed,
		initialStream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Immediately kill the failure sequence on signal
	connect(
		this, &Stream::eliminateFailureSequence,
		initialStream, &Stream::onEliminateSequence,
		Qt::DirectConnection
	);
}

void quickstreams::Stream::registerAbortionSequence(Stream* initialStream) {
	verifyAbortSeqStreamNotMember(initialStream);

	// Disconnect and eliminate (override) the registered abortion sequence
	disconnect(this, &Stream::aborted, 0, 0);
	eliminateAbortionSequence();

	// Initialize the abortion sequence and the following sequence recursively
	connect(
		this, &Stream::initializeSequences,
		initialStream, &Stream::onInitialize,
		Qt::DirectConnection
	);

	// Asynchronously awake abortion recovery sequence
	// if this stream is aborted
	connect(
		this, &Stream::aborted,
		initialStream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Immediately kill the abortion sequence on signal
	connect(
		this, &Stream::eliminateAbortionSequence,
		initialStream, &Stream::onEliminateSequence,
		Qt::DirectConnection
	);
}

void quickstreams::Stream::onPropagateFailSeqUp(Stream* initialStream) {
	registerFailureSequence(initialStream);
	propagateFailSeqUp(initialStream);
}

void quickstreams::Stream::onPropagateFailSeqDown(Stream* initialStream) {
	registerFailureSequence(initialStream);
	propagateFailSeqDown(initialStream);
}

void quickstreams::Stream::onPropagateAbortSeqUp(Stream* initialStream) {
	registerAbortionSequence(initialStream);
	propagateAbortSeqUp(initialStream);
}

void quickstreams::Stream::onPropagateAbortSeqDown(Stream* initialStream) {
	registerAbortionSequence(initialStream);
	propagateAbortSeqDown(initialStream);
}

void quickstreams::Stream::awake(
	QVariant data,
	quickstreams::Stream::WakeCondition wakeCondition
) {
	// If the stream is supposed to delay its awakening then delay it
	// but only if the wake condition allows it
	if(
		_awakeningTimer != nullptr
		&& wakeCondition != WakeCondition::DefaultNoDelay
		&& wakeCondition != WakeCondition::AbortNoDelay
	) {
		_state = State::AwaitingDelay;
		// Change wake condition to NoDelay to prevent an infinite delay loop
		switch(wakeCondition) {
		case WakeCondition::Default:
			wakeCondition = WakeCondition::DefaultNoDelay;
			break;
		case WakeCondition::Abort:
			wakeCondition = WakeCondition::AbortNoDelay;
			break;
		default:
			break;
		}
		QObject::disconnect(
			_awakeningTimer, &QTimer::timeout,
			nullptr, nullptr
		);
		QObject::connect(
			_awakeningTimer, &QTimer::timeout,
			this, [this, data, wakeCondition]() {
				awake(data, wakeCondition);
			}
		);
		_awakeningTimer->start();
		return;
	}

	// If this stream is not yet aborted but requested to abort
	// then transit to aborted state. Otherwise transite to active state
	if(_state != State::Aborted && (
		wakeCondition == WakeCondition::Abort
		|| wakeCondition == WakeCondition::AbortNoDelay
	)) {
		_state = State::Aborted;
	} else if(_state == State::Awaiting) {
		_state = State::Active;
	}
	_provider->activated();

	// if function is not callable the stream is considered closed
	if(_executable.isNull()) {
		emitClosed(QVariant());
		return;
	};
	_executable->execute(data);

	// If function returned an error the stream is considered failed
	if(_executable->hasFailed()) {
		switch(_state) {
		case State::Aborted:
			emitFailed(
				_executable->getError(),
				WakeCondition::Abort
			);
			break;
		default:
			emitFailed(
				_executable->getError(),
				WakeCondition::Default
			);
			break;
		}
	}

	// But if a stream was returned then wrap this stream.
	// There's no need for this stream to acquire ownership,
	// it's okay for the wrapped stream to execute freely.
	else if(_executable->hasReturnedStream()) {
		connect(
			_executable->stream(), &Stream::failed,
			this, [this](QVariant reason) {
				emitFailed(reason, WakeCondition::Default);
			},
			Qt::QueuedConnection
		);
		connect(
			_executable->stream(), &Stream::closed,
			this, [this](QVariant data, WakeCondition wakeCondition) {
				Q_UNUSED(wakeCondition)
				emitClosed(data);
			},
			Qt::QueuedConnection
		);
	}
}

void quickstreams::Stream::onInitialize() {
	_state = State::Awaiting;
	initializeSequences();
}

void quickstreams::Stream::onEliminateSequence() {
	die();
	eliminateSequence();
}

quickstreams::Stream::Reference quickstreams::Stream::delay(qint32 duration) {
	if(_awakeningTimer == nullptr) _awakeningTimer = new QTimer(this);
	_awakeningTimer->setInterval(duration);
	_awakeningTimer->setSingleShot(true);
	return _provider->reference(this);
}

quickstreams::Stream::Reference quickstreams::Stream::retry(
	const QVariantList& samples,
	qint32 maxTrials
) {
	_retryer.reset(new Retryer(samples, maxTrials));
	return _provider->reference(this);
}

quickstreams::Stream::Reference quickstreams::Stream::repeat(
	Repeater::Reference newRepeater
) {
	_repeater.swap(newRepeater);
	return _provider->reference(this);
}

quickstreams::Stream::Reference quickstreams::Stream::repeat(
	LambdaRepeater::Function function
) {
	_repeater.reset(new LambdaRepeater(function));
	return _provider->reference(this);
}

quickstreams::Stream::Reference quickstreams::Stream::attach(
	const Executable::Reference& executable
) {
	verifyAttach();

	auto reference(create(
		executable, Type::Abortable, CaptionStatus::Attached
	));
	_captured = Captured::Attached;
	connectSubsequent(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::attach(
	LambdaSyncExecutable::Function function
) {
	return attach(Executable::Reference(new LambdaSyncExecutable(function)));
}

quickstreams::Stream::Reference quickstreams::Stream::attach(
	const Reference& stream
) {
	verifyAttachStream(stream);

	_captured = Captured::Attached;
	connectSubsequent(stream.data());
	stream->_captionStatus = CaptionStatus::Attached;
	return stream;
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	const Executable::Reference& executable
) {
	verifyBind();

	auto reference(create(executable, Type::Abortable, CaptionStatus::Bound));
	_captured = Captured::Bound;
	connectSubsequent(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	LambdaSyncExecutable::Function function
) {
	return bind(Executable::Reference(new LambdaSyncExecutable(function)));
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	const Reference& stream
) {
	verifyBindStream(stream);

	_captured = Captured::Bound;
	connectSubsequent(stream.data());
	stream->_captionStatus = CaptionStatus::Bound;
	return stream;
}

quickstreams::Stream::Reference quickstreams::Stream::event(
	const QString& name,
	const Callback::Reference& callback
) {
	if(name.length() < 1) return _provider->reference(this);
	if(callback.isNull()) return _provider->reference(this);
	_observedEvents.insert(name, callback);
	return _provider->reference(this);
}

quickstreams::Stream::Reference quickstreams::Stream::failure(
	const Executable::Reference& executable
) {
	verifyFailureSequence();

	auto reference(create(executable, Type::Atomic, CaptionStatus::Bound));
	registerFailureSequence(reference.data());
	propagateFailSeqDown(reference.data());
	propagateFailSeqUp(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::failure(
	LambdaSyncExecutable::Function function
) {
	return failure(Executable::Reference(new LambdaSyncExecutable(function)));
}

quickstreams::Stream::Reference quickstreams::Stream::failure(
	const Reference& stream
) {
	verifyFailureSequenceStream(stream.data());

	registerFailureSequence(stream.data());
	propagateFailSeqDown(stream.data());
	propagateFailSeqUp(stream.data());
	stream->_captionStatus = CaptionStatus::Bound;
	return stream;
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	const Executable::Reference& executable
) {
	verifyAbortionSequence();

	auto reference(create(executable, Type::Atomic, CaptionStatus::Bound));
	// When this stream was aborted - awake the abortion stream
	registerAbortionSequence(reference.data());
	propagateAbortSeqDown(reference.data());
	propagateAbortSeqUp(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	LambdaSyncExecutable::Function function
) {
	return abortion(Executable::Reference(new LambdaSyncExecutable(function)));
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	const Reference& stream
) {
	verifyAbortionSequenceStream(stream.data());

	// When this stream was aborted - awake the abortion stream
	registerAbortionSequence(stream.data());
	propagateAbortSeqDown(stream.data());
	propagateAbortSeqUp(stream.data());
	stream->_captionStatus = CaptionStatus::Bound;
	return stream;
}

void quickstreams::Stream::abort() {
	// Dead, canceled, initializing and already aborted streams
	// cannot be aborted
	if(isInactive() || isAborted()) return;

	// If this stream is delayed currently awaiting its awakening
	// then cancel it in case it's attached or free.
	// Only bound streams should block until the delay is over
	if(_state == State::AwaitingDelay) {
		_state = State::Aborted;
		if(isAbortable() && _awakeningTimer != nullptr) {
			_awakeningTimer->stop();
		}
	} else {
		_state = State::Aborted;
		if(isAbortable()) abortSubordinate();
	}
}

quickstreams::Stream::State quickstreams::Stream::state() const {
	return _state;
}

bool quickstreams::Stream::isAbortable() const {
	return _type == Type::Abortable;
}

bool quickstreams::Stream::isAborted() const {
	return _state == State::Aborted;
}

bool quickstreams::Stream::isInactive() const {
	switch(_state) {
	case State::Active:
	case State::AwaitingDelay:
	case State::Aborted:
		return false;
	default:
		return true;
	}
}
