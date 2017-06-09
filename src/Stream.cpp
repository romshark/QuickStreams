#include "Stream.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "LambdaExecutable.hpp"
#include "Repeater.hpp"
#include "LambdaRepeater.hpp"
#include <QJSValue>
#include <QList>
#include <QString>
#include <QVariant>
#include <QMetaObject>
#include <QTimer>
#include <QSharedPointer>
#include <QDebug>

quickstreams::Stream::Stream(
	ProviderInterface* provider,
	const Executable::Reference& executable,
	Type type,
	CaptionStatus captionStatus
) :
	QObject(nullptr),
	_provider(provider),
	_type(type),
	_state(State::New),
	_conFlowBranching(ControlFlowBranching::None),
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

bool quickstreams::Stream::cannotAttach() const {
	//A stream cannot attach-capture multiple other streams
	if(_captured != Captured::None) {
		qWarning() << "CAUTION:"
			" Attempted to attach a stream (created from a prototype)"
			" to a stream that already captures another one!";
		return true;
	}
	return false;
}

bool quickstreams::Stream::cannotAttachStream(const Reference& stream) const {
	// A stream cannot attach-capture itself
	if(stream.data() == this) {
		qWarning() << "CAUTION: Attempted to attach a stream to itself!";
		return true;
	}

	if(cannotAttach()) return true;

	// A stream cannot attach-capture a non-free stream
	if(stream->_captionStatus != CaptionStatus::Free) {
		qWarning() << "CAUTION: Attempted to attach a non-free stream!";
		return true;
	}

	return false;
}

bool quickstreams::Stream::cannotBind() const {
	//A stream cannot bind-capture multiple other streams
	if(_captured != Captured::None) {
		qWarning() << "CAUTION:"
			" Attempted to bind a stream (created from a prototype)"
			" to a stream that already captures another one!";
		return true;
	}
	return false;
}

bool quickstreams::Stream::cannotBindStream(const Reference& stream) const {
	// A stream cannot bind-capture itself
	if(stream.data() == this) {
		qWarning() << "CAUTION: Attempted to bind a stream to itself!";
		return true;
	}

	if(cannotBind()) return true;

	// A stream cannot bind-capture a non-free stream
	if(stream->_captionStatus != CaptionStatus::Free) {
		qWarning() << "CAUTION: Attempted to bind a non-free stream!";
		return true;
	}

	return false;
}

bool quickstreams::Stream::cannotBranchFailure() const {
	// If this stream already registered a failure sequence
	// it therefore cannot register yet another one
	switch(_conFlowBranching) {
	case ControlFlowBranching::Failure:
	case ControlFlowBranching::Both:
		qWarning() << "CAUTION:"
			" Attempted to register a failure sequence"
			" to a stream that already refers to another one!";
		return true;
	default:
		break;
	}
	return false;
}

bool quickstreams::Stream::cannotBranchFailureStream(
	const Reference& stream
) const {
	// A stream cannot register itself
	if(stream.data() == this) {
		qWarning() << "CAUTION:"
			" Attempted to register a stream to itself"
			" as the initial stream of a failure sequence!";
		return true;
	}

	if(cannotBranchAbortion()) return true;

	// A stream cannot be captured by multiple individual streams
	if(stream->_captionStatus != CaptionStatus::Free) {
		qWarning() << "CAUTION:"
			" Attempted to register a non-free stream"
			" as the initial stream of a failure sequence!";
		return true;
	}

	return false;
}

bool quickstreams::Stream::cannotBranchAbortion() const {
	// If this stream already registered an abortion sequence
	// it therefore cannot register yet another one
	switch(_conFlowBranching) {
	case ControlFlowBranching::Abortion:
	case ControlFlowBranching::Both:
		qWarning() << "CAUTION:"
			" Attempted to register an abortion sequence"
			" to a stream that already refers to another one!";
		return true;
	default:
		break;
	}
	return false;
}

bool quickstreams::Stream::cannotBranchAbortionStream(
	const Reference& stream
) const {
	// A stream cannot register itself
	if(stream.data() == this) {
		qWarning() << "CAUTION:"
			" Attempted to register a stream to itself"
			" as the initial stream of an abortion sequence!";
		return true;
	}

	if(cannotBranchAbortion()) return true;

	// A stream cannot be captured by multiple individual streams
	if(stream->_captionStatus != CaptionStatus::Free) {
		qWarning() << "CAUTION:"
			" Attempted to register a non-free stream"
			" as the initial stream of an abortion sequence!";
		return true;
	}

	return false;
}

void quickstreams::Stream::verifyFailureSequenceStream(Stream* stream) const {
	if(this != stream) return;
	qWarning() << "CAUTION:"
		" Registered a sequence member stream as the initial stream"
		" of a failure recovery sequence branching off it!";
}

void quickstreams::Stream::verifyAbortionSequenceStream(Stream* stream) const {
	if(this != stream) return;
	qWarning() << "CAUTION:"
		" Registered a sequence member stream as the initial stream"
		" of an abortion recovery sequence branching off it!";
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
	// Receive failure and abortion stream propagation signals
	// during the declaration
	connect(
		stream, &Stream::propagateFailureStream,
		this, &Stream::registerFailureSequence,
		Qt::DirectConnection
	);
	connect(
		stream, &Stream::propagateAbortionStream,
		this, &Stream::registerAbortionSequence,
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
	switch(_state) {
	case State::New:
		_state = State::Canceled;
		break;
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
	awake(QVariant(), WakeCondition::Default);
}

void quickstreams::Stream::registerFailureSequence(
	Stream* failureStream
) {
	verifyFailureSequenceStream(failureStream);

	// Asynchronously awake failure recovery stream if this stream fails
	connect(
		this, &Stream::failed,
		failureStream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Immediately kill the failure sequence on signal
	connect(
		this, &Stream::eliminateFailureSequence,
		failureStream, &Stream::onEliminateSequence,
		Qt::DirectConnection
	);

	// Register control flow branching to either both (failure & abortion)
	// or failure sequence only. At this point control flow branching
	// can neither be both nor failure only since this is prevented
	// in the failure operator
	if(_conFlowBranching == ControlFlowBranching::Abortion) {
		_conFlowBranching = ControlFlowBranching::Both;
	} else {
		_conFlowBranching = ControlFlowBranching::Failure;
	}

	// Propagate failure stream to superordinate streams
	propagateFailureStream(failureStream);
}

void quickstreams::Stream::registerAbortionSequence(
	Stream* abortionStream
) {
	verifyAbortionSequenceStream(abortionStream);

	// Asynchronously awake abortion recovery stream if this stream is aborted
	connect(
		this, &Stream::aborted,
		abortionStream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Immediately kill the abortion sequence on signal
	connect(
		this, &Stream::eliminateAbortionSequence,
		abortionStream, &Stream::onEliminateSequence,
		Qt::DirectConnection
	);

	// Register control flow branching to either both (failure & abortion)
	// or abortion sequence only. At this point control flow branching
	// can neither be both nor abortion only since this is prevented
	// in the abortion operator
	if(_conFlowBranching == ControlFlowBranching::Failure) {
		_conFlowBranching = ControlFlowBranching::Both;
	} else {
		_conFlowBranching = ControlFlowBranching::Abortion;
	}

	// Propagate abortion stream to superordinate streams
	propagateAbortionStream(abortionStream);
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
	} else if(_state == State::New) {
		_state = State::Active;
	}

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
	if(cannotAttach()) return _provider->reference(this);

	auto reference(create(
		executable, Type::Abortable, CaptionStatus::Attached
	));
	_captured = Captured::Attached;
	connectSubsequent(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::attach(
	LambdaExecutable::Function prototype
) {
	return attach(Executable::Reference(new LambdaExecutable(prototype)));
}

quickstreams::Stream::Reference quickstreams::Stream::attach(
	const Reference& stream
) {
	if(cannotAttachStream(stream)) return _provider->reference(this);

	_captured = Captured::Attached;
	connectSubsequent(stream.data());
	stream->_captionStatus = CaptionStatus::Attached;
	return stream;
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	const Executable::Reference& executable
) {
	if(cannotBind()) return _provider->reference(this);

	auto reference(create(executable, Type::Abortable, CaptionStatus::Bound));
	_captured = Captured::Bound;
	connectSubsequent(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	LambdaExecutable::Function prototype
) {
	return bind(Executable::Reference(new LambdaExecutable(prototype)));
}

quickstreams::Stream::Reference quickstreams::Stream::bind(
	const Reference& stream
) {
	if(cannotBindStream(stream)) return _provider->reference(this);

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
	if(cannotBranchFailure()) return _provider->reference(this);

	auto reference(create(executable, Type::Atomic, CaptionStatus::Bound));
	// When this stream failed - awake the failure stream
	registerFailureSequence(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::failure(
	LambdaExecutable::Function prototype
) {
	return failure(Executable::Reference(new LambdaExecutable(prototype)));
}

quickstreams::Stream::Reference quickstreams::Stream::failure(
	const Reference& stream
) {
	if(cannotBranchFailureStream(stream)) return _provider->reference(this);

	// When this stream failed - awake the failure stream
	registerFailureSequence(stream.data());
	stream->_captionStatus = CaptionStatus::Bound;
	return stream;
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	const Executable::Reference& executable
) {
	if(cannotBranchAbortion()) return _provider->reference(this);

	auto reference(create(executable, Type::Atomic, CaptionStatus::Bound));
	// When this stream was aborted - awake the abortion stream
	registerAbortionSequence(reference.data());
	return reference;
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	LambdaExecutable::Function prototype
) {
	return abortion(Executable::Reference(new LambdaExecutable(prototype)));
}

quickstreams::Stream::Reference quickstreams::Stream::abortion(
	const Reference& stream
) {
	if(cannotBranchAbortionStream(stream)) return _provider->reference(this);

	// When this stream was aborted - awake the abortion stream
	registerAbortionSequence(stream.data());
	stream->_captionStatus = CaptionStatus::Bound;
	return stream;
}

void quickstreams::Stream::abort() {
	// Dead, canceled and already aborted streams cannot be aborted
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

bool quickstreams::Stream::isAbortable() const {
	return _type == Type::Abortable;
}

bool quickstreams::Stream::isAborted() const {
	return _state == State::Aborted;
}

bool quickstreams::Stream::isInactive() const {
	switch(_state) {
	case State::Dead:
	case State::Canceled:
	case State::New:
		return true;
	default:
		return false;
	}
}
