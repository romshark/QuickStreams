#include "QmlStream.hpp"
#include "Stream.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "JsExecutable.hpp"
#include "JsCallback.hpp"
#include "JsRepeater.hpp"
#include "Provider.hpp"
#include <QJSValue>
#include <QString>
#include <QVariant>
#include <QtQml>
#include <QMetaObject>
#include <QTimer>
#include <QSharedPointer>

quickstreams::qml::QmlStream::QmlStream(
	QQmlEngine* engine,
	quickstreams::Stream::Reference reference
) :
	QObject(nullptr),
	_engine(engine),
	_reference(reference),
	_handle(
		&_reference->_handle,
		// Called when qml stream is requested to adopt another qml stream
		[this](QmlStream* another) {
			if(!another) {
				another = new QmlStream(
					_engine,
					_reference->create(
						nullptr,
						quickstreams::Stream::Type::Atomic,
						quickstreams::Stream::Capture::Bound
					)
				);
			}
			_reference->adopt(another->_reference);
			return another;
		},
		// Called when qmlstream reference is requested
		[this]() {
			return this;
		}
	)
{}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::fromJsValue(
	const QJSValue& value,
	quickstreams::Stream::Type type,
	quickstreams::Stream::Capture capture
) {
	if(value.isCallable()) {
		// Return a newly created stream wrapper based on a new stream
		auto jsExec(new JsExecutable(_engine, value));
		auto stream(new QmlStream(_engine, _reference->create(
			Executable::Reference(jsExec), type, capture
		)));
		jsExec->setQmlHandle(&stream->_handle);
		return stream;
	} else if(value.toVariant().canConvert<QmlStream*>()) {
		// Return a newly created stream wrapper based on another wrapper
		return qjsvalue_cast<QmlStream*>(value);
	}
	return new QmlStream(_engine, _reference->create(nullptr, type, capture));
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::delay(
	const QJSValue& duration
) {
	if(!duration.isNumber()) return this;
	_reference->delay(duration.toInt());
	return this;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::retry(
	const QVariant& samples,
	const QJSValue& maxTrials
) {
	QVariantList errorSamples;
	if(samples.canConvert<QVariantList>()) {
		errorSamples = samples.value<QVariantList>();
	} else {
		errorSamples.append(samples);
	}
	int trials(-1);
	if(maxTrials.isNumber()) {
		trials = maxTrials.toInt();
	}
	_reference->retry(errorSamples, trials);
	return this;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::repeat(
	const QJSValue& condition
) {
	if(!condition.isCallable()) return this;
	_reference->repeat(JsRepeater::Reference(new JsRepeater(condition)));
	return this;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::attach(
	const QJSValue& target
) {
	//TODO: forbid declaration of multiple subsequent streams
	auto stream(fromJsValue(
		target,
		quickstreams::Stream::Type::Abortable,
		quickstreams::Stream::Capture::Attached
	));
	_reference->attach(stream->_reference);
	return stream;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::bind(
	const QJSValue& target
) {
	//TODO: forbid declaration of multiple subsequent streams
	auto stream(fromJsValue(
		target,
		quickstreams::Stream::Type::Abortable,
		quickstreams::Stream::Capture::Bound
	));
	_reference->bind(stream->_reference);
	return stream;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::event(
	const QVariant& name,
	const QJSValue& callback
) {
	if(!name.canConvert<QString>()) return this;
	_reference->event(
		name.toString(),
		Callback::Reference(new JsCallback(_engine, callback))
	);
	return this;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::failure(
	const QJSValue& target
) {
	auto stream(fromJsValue(
		target,
		quickstreams::Stream::Type::Atomic,
		quickstreams::Stream::Capture::Bound
	));
	_reference->failure(stream->_reference);
	return stream;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::abortion(
	const QJSValue& target
) {
	auto stream(fromJsValue(
		target,
		quickstreams::Stream::Type::Atomic,
		quickstreams::Stream::Capture::Bound
	));
	_reference->abortion(stream->_reference);
	return stream;
}

void quickstreams::qml::QmlStream::abort() {
	_reference->abort();
}

bool quickstreams::qml::QmlStream::isAbortable() const {
	return _reference->isAbortable();
}

bool quickstreams::qml::QmlStream::isAborted() const {
	return _reference->isAborted();
}

static void registerQmlTypes() {
    qmlRegisterInterface<quickstreams::qml::QmlStream>("QmlStream");
}

Q_COREAPP_STARTUP_FUNCTION(registerQmlTypes)
