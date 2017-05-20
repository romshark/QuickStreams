#include "Stream.hpp"
#include "StreamHandle.hpp"
#include <QQmlEngine>
#include <QJSValue>
#include <QString>
#include <QVariant>
#include <QtQml>
#include <QMetaObject>

quickstreams::Stream::Stream(
	QQmlEngine* engine,
	const QJSValue& function,
	QObject* parent
) :
	QObject(parent),
	_engine(engine),
	_dead(true),
	_function(function),
	_maxTrials(-2),
	_currentTrial(0),
	_handle(
		// when stream is requested to emit an event
		[this](const QString& name, const QVariant& data) mutable {
			emitEvent(name, data);
		},
		// when stream is requested to close
		[this](const QVariant& reason) mutable {
			emitClosed(reason);
		},
		// when stream is requested to fail
		[this](const QVariant& reason) mutable {
			emitFailed(reason);
		},
		// when stream is requested to await another stream
		[this](const QVariant& stream) mutable {
			if(!stream.canConvert<Stream*>()) return false;
			Stream* streamToAwait(stream.value<Stream*>());
			connect(
				streamToAwait, &Stream::failed,
				this, &Stream::handleAwaitFailed,
				Qt::QueuedConnection
			);
			connect(
				streamToAwait, &Stream::closed,
				this, &Stream::handleAwaitClosed,
				Qt::QueuedConnection
			);
			return true;
		}
	)
{}

void quickstreams::Stream::emitEvent(
	const QString& name,
	const QVariant& data
) {
	// dead channels can't emit any events
	if(_dead) return;
	eventEmitted(name, data);
}

void quickstreams::Stream::emitClosed(const QVariant& data) {
	// dead channels can't be closed
	if(_dead) return;
	// reset trial counter on success
	_currentTrial = 0;
	// check whether repeat is desired
	QJSValue result(_repeatCondition.call());
	if(result.isBool() && result.toBool() == true) {
		QMetaObject::invokeMethod(this, "awake", Qt::QueuedConnection);
		return;
	}
	// otherwise close this stream
	closed(data);
	_dead = true;
}

void quickstreams::Stream::emitFailed(const QVariant& data) {
	// dead channels can't fail
	if(_dead) return;
	_currentTrial++;
	// check whether retrial is desired
	if(
		(_maxTrials < 0 || _currentTrial <= _maxTrials)
		&& verifyErrorListed(data)
	) {
		QMetaObject::invokeMethod(this, "awake", Qt::QueuedConnection);
		return;
	}
	// otherwise fail this stream
	failed(data);
	_dead = true;
}

bool quickstreams::Stream::verifyErrorListed(const QVariant& err) const {
	for(
		QVariantList::const_iterator itr(_retryErrSamples.constBegin());
		itr != _retryErrSamples.constEnd();
		itr++
	) if(err == *itr) return true;
	return false;
}

quickstreams::Stream* quickstreams::Stream::createSubsequentStream(
	const QJSValue& callback
) const {
	if(callback.isCallable()) {
		// create new streams to wrap function objects
		return new Stream(_engine, callback);
	} else if(
		callback.toVariant().userType() == qMetaTypeId<quickstreams::Stream*>()
	) {
		// other streams are simply connected
		return qjsvalue_cast<Stream*>(callback);
	}

	// in all other cases create a new stream with a null funtion
	return new Stream(_engine, QJSValue());
}

void quickstreams::Stream::handleFailureStreamPropagation(Stream* failureStream) {
	connect(
		this, &Stream::failed,
		failureStream, &Stream::awake,
		Qt::QueuedConnection
	);
	// propagate failure stream to superordinate streams
	propagateFailureStream(failureStream);
}

void quickstreams::Stream::awake(QVariant data) {
	_dead = false;
	// if function is not callable the stream is considered closed
	if(!_function.isCallable()) {
		emitClosed(QVariant());
		return;
	};
	QJSValue result(_function.call({
		_engine->toScriptValue(_handle),
		_engine->toScriptValue(data)
	}));
	// if function returned an error the stream is considered failed
	if(result.isError()) {
		emitFailed(QVariant::fromValue<QJSValue>(result));
	}
}

void quickstreams::Stream::awakeFromEvent(QString name, QVariant data) {
	// verify that this event is one of those
	// which this stream is listening for
	if(!_observedEvents.contains(name)) return;

	awake(data);
}

void quickstreams::Stream::handleAwaitClosed(QVariant data) {
	emitClosed(data);
}

void quickstreams::Stream::handleAwaitFailed(QVariant reason) {
	emitFailed(reason);
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
	// if given amount of trials is negative, make it 0
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

quickstreams::Stream* quickstreams::Stream::next(const QJSValue& target) {
	Stream* stream(createSubsequentStream(target));
	// when this stream closes - awake the next
	connect(
		this, &Stream::closed,
		stream, &Stream::awake,
		Qt::QueuedConnection
	);
	// receive failure stream propagation signals during the declaration
	connect(
		stream, &Stream::propagateFailureStream,
		this, &Stream::handleFailureStreamPropagation,
		Qt::DirectConnection
	);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::event(
	const QVariant& name,
	QJSValue target
) {
	if(!name.canConvert<QString>()) {
		throw;
	}
	Stream* stream(createSubsequentStream(target));
	// when this stream emits an event - awake the subsequent stream
	stream->_observedEvents.insert(name.value<QString>());
	connect(
		this, &Stream::eventEmitted,
		stream, &Stream::awakeFromEvent,
		Qt::QueuedConnection
	);
	return stream;
}

quickstreams::Stream* quickstreams::Stream::failure(const QJSValue& target) {
	Stream* stream(createSubsequentStream(target));
	// when this stream failed - awake the failure stream
	handleFailureStreamPropagation(stream);
	return stream;
}

static void registerQmlTypes() {
    qmlRegisterInterface<quickstreams::Stream>("Stream");
}

Q_COREAPP_STARTUP_FUNCTION(registerQmlTypes)
