#include "QmlProvider.hpp"
#include "Stream.hpp"
#include "QmlStream.hpp"
#include "Provider.hpp"
#include "JsExecutable.hpp"
#include "Stream.hpp"
#include <QObject>
#include <QQmlEngine>
#include <QJSValue>

quickstreams::qml::QmlProvider::QmlProvider(
	QQmlEngine* engine,
	Provider* provider
) :
	QObject(provider),
	_provider(provider),
	_engine(engine)
{
	connect(
		provider, &Provider::totalCreatedChanged,
		this, &QmlProvider::totalCreatedChanged
	);
	connect(
		provider, &Provider::totalExistingChanged,
		this, &QmlProvider::totalExistingChanged
	);
	connect(
		provider, &Provider::totalActiveChanged,
		this, &QmlProvider::totalActiveChanged
	);
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlProvider::toQml(
	const quickstreams::Stream::Reference& stream
) {
	if(stream.isNull()) return nullptr;
	return new QmlStream(_engine, stream);
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlProvider::create(
	const QJSValue& target,
	quickstreams::Stream::Type type
) {
	// If target is not callable then there's no executable
	if(!target.isCallable()) {
		return new QmlStream(_engine, _provider->internalCreate(
			Executable::Reference(nullptr), type)
		);
	}

	// Otherwise create executable out of a js function
	auto jsExec(new JsExecutable(_engine, target));
	auto stream(new QmlStream(_engine, _provider->internalCreate(
		Executable::Reference(jsExec), type
	)));

	// Copy QML stream handle to the independent JavaScript executable
	jsExec->_qmlHandle = stream->_handle;
	return stream;
}

quickstreams::Stream::Type quickstreams::qml::QmlProvider::Atomic() const {
	return quickstreams::Stream::Type::Atomic;
}

quickstreams::Stream::Type quickstreams::qml::QmlProvider::Abortable() const {
	return quickstreams::Stream::Type::Abortable;
}

quint64 quickstreams::qml::QmlProvider::totalCreated() const {
	return _provider->totalCreated();
}

quint64 quickstreams::qml::QmlProvider::totalExisting() const {
	return _provider->totalExisting();
}

quint64 quickstreams::qml::QmlProvider::totalActive() const {
	return _provider->totalActive();
}
