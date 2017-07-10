#include "QmlProvider.hpp"
#include "Stream.hpp"
#include "QmlStream.hpp"
#include "Provider.hpp"
#include "JsExecutable.hpp"
#include "Stream.hpp"
#include <QObject>
#include <QQmlEngine>
#include <QJSValue>
#include <QString>
#include <QVariant>
#include <QCoreApplication>
#include <QMetaType>

int quickstreams::qml::ExceptionTypeList::Exception() {
	return quickstreams::exception::Exception::type();
}

int quickstreams::qml::ExceptionTypeList::JsException() {
	return quickstreams::exception::JsException::type();
}

int quickstreams::qml::ExceptionTypeList::BadTypeId() {
	return quickstreams::exception::BadTypeId::type();
}

int quickstreams::qml::ExceptionTypeList::BadCast() {
	return quickstreams::exception::BadCast::type();
}

int quickstreams::qml::ExceptionTypeList::BadWeakPtr() {
	return quickstreams::exception::BadWeakPtr::type();
}

int quickstreams::qml::ExceptionTypeList::BadFunctionCall() {
	return quickstreams::exception::BadFunctionCall::type();
}

int quickstreams::qml::ExceptionTypeList::BadAlloc() {
	return quickstreams::exception::BadAlloc::type();
}

int quickstreams::qml::ExceptionTypeList::BadArrayNewLength() {
	return quickstreams::exception::BadArrayNewLength::type();
}

int quickstreams::qml::ExceptionTypeList::BadException() {
	return quickstreams::exception::BadException::type();
}

int quickstreams::qml::ExceptionTypeList::InvalidArgument() {
	return quickstreams::exception::InvalidArgument::type();
}

int quickstreams::qml::ExceptionTypeList::DomainError() {
	return quickstreams::exception::DomainError::type();
}

int quickstreams::qml::ExceptionTypeList::LengthError() {
	return quickstreams::exception::LengthError::type();
}

int quickstreams::qml::ExceptionTypeList::OutOfRange() {
	return quickstreams::exception::OutOfRange::type();
}

int quickstreams::qml::ExceptionTypeList::FutureError() {
	return quickstreams::exception::FutureError::type();
}

int quickstreams::qml::ExceptionTypeList::LogicError() {
	return quickstreams::exception::LogicError::type();
}

int quickstreams::qml::ExceptionTypeList::RuntimeError() {
	return quickstreams::exception::RuntimeError::type();
}

int quickstreams::qml::ExceptionTypeList::RangeError() {
	return quickstreams::exception::RangeError::type();
}

int quickstreams::qml::ExceptionTypeList::OverflowError() {
	return quickstreams::exception::OverflowError::type();
}

int quickstreams::qml::ExceptionTypeList::UnderflowError() {
	return quickstreams::exception::UnderflowError::type();
}

int quickstreams::qml::ExceptionTypeList::RegexError() {
	return quickstreams::exception::RegexError::type();
}

int quickstreams::qml::ExceptionTypeList::SystemError() {
	return quickstreams::exception::SystemError::type();
}

quickstreams::qml::ExceptionTypeList
quickstreams::qml::QmlProvider::exceptions() const {
	return exceptionTypes;
}

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

static void __register_quickstreams_qml_provider() {
    qRegisterMetaType<quickstreams::qml::ExceptionTypeList>();
}

Q_COREAPP_STARTUP_FUNCTION(__register_quickstreams_qml_provider)
