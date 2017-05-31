#include "Stream.hpp"
#include "StreamHandle.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QString>
#include <QVariant>
#include <QtQml>
#include <QMetaObject>
#include <QTimer>

quickstreams::Stream::Stream(
	QQmlEngine* engine,
	const QJSValue& function,
	Type type,
	Belonging belonging,
	QObject* parent
) :
	QObject(parent),
	_engine(engine),
	_type(type),
	_state(State::Dead),
	_belonging(belonging),
	_function(function),
	_parent(nullptr),
	_nextType(NextType::None),
	_awakeningTimer(nullptr),
	_maxTrials(-2),
	_currentTrial(0),
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
		[this](const QVariant& target) {
			// If target is invalid then create a new stream
			Stream* stream;
			if(target.canConvert<Stream*>()) {
				stream = target.value<Stream*>();
			} else {
				stream = new Stream(
					_engine,
					QJSValue(),
					Type::Atomic,
					Belonging::Bound
				);
			}
			// Acquire full ownership
			stream->setParentStream(this);
			return stream;
		},
		// Called when stream reference is requested
		[this]() {
			return this;
		},
		// Called when isAbortable is requested
		[this]() {
			return this->isAbortable();
		},
		// Called when isAborted is requested
		[this]() {
			return this->isAborted();
		}
	)
{
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

void quickstreams::Stream::emitEvent(
	const QString& name,
	const QVariant& data
) {
	// Dead channels can't emit any events
	if(_state == State::Dead) return;
	eventEmitted(name, data);
}

void quickstreams::Stream::emitClosed(const QVariant& data) {
	// Dead channels can't be closed
	if(_state == State::Dead) return;

	// Reset trial counter on success
	_currentTrial = 0;

	// Check whether repeat is desired
	QJSValue result(_repeatCondition.call({
		QJSValue(isAborted())
	}));

	if(result.isBool() && result.toBool() == true) {
		// Repeat asynchronously resurrecting this stream in another tick
		repeatIteration(data, WakeCondition::Default);
		return;
	}

	// If there is no next stream but this stream was aborted
	// the abortion recovery stream should be invoked next.
	// Otherwise execute the subsequent bound stream
	// with the condition that its state is initially 'Aborted'
	if(_state == State::Aborted) {
		switch(_nextType) {
		case NextType::Bound:
			closed(data, WakeCondition::Abort);
			return;
		default:
			aborted(data, WakeCondition::Default);
			return;
		}
	}

	// Otherwise close this stream initializing the next stream
	closed(data, WakeCondition::Default);
	_state = State::Dead;
}

void quickstreams::Stream::emitFailed(
	const QVariant& data,
	WakeCondition wakeCondition
) {
	// Dead channels can't fail
	if(_state == State::Dead) return;
	_currentTrial++;
	// Check whether retrial is desired
	if(
		(_maxTrials < 0 || _currentTrial <= _maxTrials)
		&& verifyErrorListed(data)
	) {
		// Retry asynchronously
		retryIteration(data, wakeCondition);
		return;
	}
	// Otherwise fail this stream and invoke failure recovery stream
	failed(data, WakeCondition::Default);
	_state = State::Dead;
}

bool quickstreams::Stream::verifyErrorListed(const QVariant& err) const {
	for(
		QVariantList::const_iterator itr(_retryErrSamples.constBegin());
		itr != _retryErrSamples.constEnd();
		itr++
	) if(err == *itr) return true;
	return false;
}

quickstreams::Stream* quickstreams::Stream::subsequentStream(
	const QJSValue& target,
	Type streamType,
	Belonging belonging
) {
	Stream* stream;
	if(target.isCallable()) {
		// Create new streams to wrap function objects
		stream = new Stream(_engine, target, streamType, belonging);
	} else if(target.toVariant().canConvert<Stream*>()) {
		// Passed streams are connected
		stream = qjsvalue_cast<Stream*>(target);

		// Take ownership to prevent free execution
		stream->_belonging = belonging;
	} else {
		// In all other cases create a new stream with a null function
		stream = new Stream(_engine, QJSValue(), streamType, belonging);
	}
	return stream;
}

void quickstreams::Stream::setParentStream(Stream *parentStream) {
	// Remember parent stream for automatic inheritance
	_parent = parentStream;

	// Immediately react to parents abortion
	connect(
		_parent, &Stream::abortChildren,
		this, &Stream::abort,
		Qt::DirectConnection
	);
}

void quickstreams::Stream::connectSubsequent(Stream* stream) {
	// Receive failure and abortion stream propagation signals
	// during the declaration
	connect(
		stream, &Stream::propagateFailureStream,
		this, &Stream::registerFailureRecoveryStream,
		Qt::DirectConnection
	);
	connect(
		stream, &Stream::propagateAbortionStream,
		this, &Stream::registerAbortionRecoveryStream,
		Qt::DirectConnection
	);

	// When this stream closes - awake the attached stream
	connect(
		this, &Stream::closed,
		stream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Automatically inherit parent stream
	if(_parent) stream->setParentStream(_parent);
}

void quickstreams::Stream::initialize() {
	if(_belonging != Belonging::Free) return;
	awake(QVariant(), WakeCondition::Default);
}

void quickstreams::Stream::registerFailureRecoveryStream(
	Stream* failureStream
) {
	// Asynchronously awake failure recovery stream if this stream fails
	connect(
		this, &Stream::failed,
		failureStream, &Stream::awake,
		Qt::QueuedConnection
	);

	// Propagate failure stream to superordinate streams
	propagateFailureStream(failureStream);
}

void quickstreams::Stream::registerAbortionRecoveryStream(
	Stream* abortionStream
) {
	// Asynchronously awake abortion recovery stream if this stream is aborted
	connect(
		this, &Stream::aborted,
		abortionStream, &Stream::awake,
		Qt::QueuedConnection
	);

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

	// If this stream is not yet aborted or requested to abort
	// then mark it as active. Otherwise mark as aborted
	if(_state != State::Aborted && (
		wakeCondition == WakeCondition::Abort
		|| wakeCondition == WakeCondition::AbortNoDelay
	)) {
		_state = State::Aborted;
	} else if(_state == State::Dead) {
		_state = State::Active;
	}

	// if function is not callable the stream is considered closed
	if(!_function.isCallable()) {
		emitClosed(QVariant());
		return;
	};
	QJSValue result(_function.call({
		_engine->toScriptValue(_handle),
		_engine->toScriptValue(data)
	}));

	// If function returned an error the stream is considered failed
	if(result.isError()) {
		switch(_state) {
		case State::Aborted:
			emitFailed(
				QVariant::fromValue<QJSValue>(result),
				WakeCondition::Abort
			);
			break;
		default:
			emitFailed(
				QVariant::fromValue<QJSValue>(result),
				WakeCondition::Default
			);
			break;
		}
	}

	// But if a stream was returned then wrap this stream.
	// There's no need for this stream to acquire ownership,
	// it's okay for the wrapped stream to execute freely.
	else if(result.toVariant().canConvert<Stream*>()) {
		auto stream(qjsvalue_cast<Stream*>(result));
		connect(
			stream, &Stream::failed,
			this, [this](QVariant reason) {
				emitFailed(reason, WakeCondition::Default);
			},
			Qt::QueuedConnection
		);
		connect(
			stream, &Stream::closed,
			this, [this](QVariant data, WakeCondition wakeCondition) {
				Q_UNUSED(wakeCondition)
				emitClosed(data);
			},
			Qt::QueuedConnection
		);
	}
}

void quickstreams::Stream::awakeFromEvent(QString name, QVariant data) {
	// Verify that this event is one of those
	// which this stream is listening for
	// in case there is at least one observed event
	if(_observedEvents.size() > 0 && !_observedEvents.contains(name)) return;
	awake(data, WakeCondition::Default);
}

quickstreams::Stream* quickstreams::Stream::delay(const QJSValue& duration) {
	if(!duration.isNumber()) return this;
	if(_awakeningTimer == nullptr) {
		_awakeningTimer = new QTimer(this);
	}
	_awakeningTimer->setInterval(duration.toInt());
	_awakeningTimer->setSingleShot(true);
	return this;
}

quickstreams::Stream* quickstreams::Stream::retry(
	const QVariant& samples,
	const QJSValue& maxTrials
) {
	_retryErrSamples.clear();
	if(samples.canConvert<QVariantList>()) {
		_retryErrSamples = samples.value<QVariantList>();
	} else {
		_retryErrSamples.append(samples);
	}
	// If given amount of trials is negative, make it 0
	// because -1 stands for infinite amount of trials
	// which is only applied when no amount of trials was set.
	if(maxTrials.isNumber()) {
		qint32 trials = maxTrials.toInt();
		if(trials < 0) trials = 0;
		_maxTrials = trials;
	}
	return this;
}

quickstreams::Stream* quickstreams::Stream::repeat(const QJSValue& condition) {
	if(!condition.isCallable()) return this;
	_repeatCondition = condition;
	return this;
}

quickstreams::Stream* quickstreams::Stream::attach(const QJSValue& target) {
	//TODO: forbid declaration of multiple subsequent streams
	_nextType = NextType::Attached;
	auto stream(subsequentStream(target, Type::Abortable, Belonging::Attached));
	connectSubsequent(stream);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::bind(const QJSValue& target) {
	//TODO: forbid declaration of multiple subsequent streams
	_nextType = NextType::Bound;
	auto stream(subsequentStream(target, Type::Abortable, Belonging::Bound));
	connectSubsequent(stream);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::event(
	const QVariant& name,
	QJSValue target
) {
	auto stream(subsequentStream(target, Type::Atomic, Belonging::Attached));

	// If event name is recognizable
	if(name.canConvert<QString>()) {
		stream->_observedEvents.insert(name.value<QString>());
	}

	// When this stream emits an event - awake the subsequent stream
	connect(
		this, &Stream::eventEmitted,
		stream, &Stream::awakeFromEvent,
		Qt::QueuedConnection
	);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::failure(const QJSValue& target) {
	auto stream(subsequentStream(target, Type::Atomic, Belonging::Bound));

	// When this stream failed - awake the failure stream
	registerFailureRecoveryStream(stream);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::abortion(const QJSValue& target) {
	auto stream(subsequentStream(target, Type::Atomic, Belonging::Bound));

	// When this stream was aborted - awake the abortion stream
	registerAbortionRecoveryStream(stream);
	return stream;
}

void quickstreams::Stream::abort() {
	// Dead streams and already aborted streams cannot be aborted
	if(_state == State::Dead || _state == State::Aborted) return;

	// If this stream is delayed currently awaiting its awakening
	// then cancel it in case it's attached or free.
	// Only bound streams should block until the delay is over
	if(_state == State::AwaitingDelay) {
		_state = State::Aborted;
		if(_type == Type::Abortable) {
			_awakeningTimer->stop();
		}
	} else {
		_state = State::Aborted;
		if(_type == Type::Abortable) {
			abortChildren();
		}
	}
}

bool quickstreams::Stream::isAbortable() const {
	if(_type == Type::Abortable) return true;
	return false;
}

bool quickstreams::Stream::isAborted() const {
	return _state == State::Aborted;
}

static void registerQmlTypes() {
    qmlRegisterInterface<quickstreams::Stream>("Stream");
}

Q_COREAPP_STARTUP_FUNCTION(registerQmlTypes)
