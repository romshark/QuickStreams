#include "QmlStream.hpp"
#include "Stream.hpp"
#include "StreamHandle.hpp"
#include "Executable.hpp"
#include "JsExecutable.hpp"
#include "JsCallback.hpp"
#include "JsRepeater.hpp"
#include "ProviderInterface.hpp"
#include <QJSValue>
#include <QString>
#include <QVariant>
#include <QtQml>
#include <QMetaObject>
#include <QTimer>
#include <QSharedPointer>

quickstreams::qml::StreamConversion::StreamConversion(
	QmlStream* stream,
	bool fromExisting
) :
	stream(stream),
	fromExisting(fromExisting)
{}

void quickstreams::qml::StreamConversion::cleanUp(
	ProviderInterface* provider
) {
	if(fromExisting) return;

	// Delete the created but unreachable stream including its wrapper
	provider->dispose(stream->_reference.data());
	stream->deleteLater();
}

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
						quickstreams::Stream::CaptionStatus::Bound
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

quickstreams::qml::StreamConversion quickstreams::qml::QmlStream::fromJsValue(
	const QJSValue& value,
	quickstreams::Stream::Type type
) {
	if(value.isCallable()) {
		// Return a newly created wrapper wrapping a new stream
		auto jsExec(new JsExecutable(_engine, value));
		auto stream(new QmlStream(_engine, _reference->create(
			Executable::Reference(jsExec), type,
			quickstreams::Stream::CaptionStatus::Free
		)));
		jsExec->setQmlHandle(&stream->_handle);
		return StreamConversion(stream);
	} else if(value.toVariant().canConvert<QmlStream*>()) {
		// Return a wrapper to an existing stream
		return StreamConversion(qjsvalue_cast<QmlStream*>(value), true);
	}

	//Otherwise return a newly created wrapper wrapping a new (dry) stream
	return StreamConversion(new QmlStream(_engine, _reference->create(
		nullptr, type, quickstreams::Stream::CaptionStatus::Free
	)));
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
	auto conversion(fromJsValue(
		target,
		quickstreams::Stream::Type::Abortable
	));

	// If the referenced stream returns itself from the attach operator then
	// there was an error, thus return this wrapper instead of the other one
	if(_reference == _reference->attach(conversion.stream->_reference)) {
		conversion.cleanUp(_reference->_provider);
		return this;
	}
	return conversion.stream;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::bind(
	const QJSValue& target
) {
	auto conversion(fromJsValue(
		target,
		quickstreams::Stream::Type::Abortable
	));

	// If the referenced stream returns itself from the bind operator then
	// there was an error, thus return this wrapper instead of the other one
	if(_reference == _reference->bind(conversion.stream->_reference)) {
		conversion.cleanUp(_reference->_provider);
		return this;
	}
	return conversion.stream;
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
	auto conversion(fromJsValue(
		target,
		quickstreams::Stream::Type::Atomic
	));

	// If the referenced stream returns itself from the failure operator then
	// there was an error, thus return this wrapper instead of the other one
	if(_reference == _reference->failure(conversion.stream->_reference)) {
		conversion.cleanUp(_reference->_provider);
		return this;
	}
	return conversion.stream;
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStream::abortion(
	const QJSValue& target
) {
	auto conversion(fromJsValue(
		target,
		quickstreams::Stream::Type::Atomic
	));

	// If the referenced stream returns itself from the abortion operator then
	// there was an error, thus return this wrapper instead of the other one
	if(_reference == _reference->abortion(conversion.stream->_reference)) {
		conversion.cleanUp(_reference->_provider);
		return this;
	}
	return conversion.stream;
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
