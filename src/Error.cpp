#include "Error.hpp"
#include <exception>
#include <future>
#include <regex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QJSValue>
#include <QCoreApplication>
#include <QMetaType>

quickstreams::exception::Exception::Exception() : QObject(nullptr) {}
quickstreams::exception::Exception::Exception(const QString& msg) :
	QObject(nullptr),
	_msg(msg)
{}

QString quickstreams::exception::Exception::message() const {
	return _msg;
}

quickstreams::exception::JsException::JsException() {}
quickstreams::exception::JsException::JsException(
	const QString& name,
	const QString& msg,
	const QVariant& data
) :
	Exception(msg),
	_name(name),
	_data(data)
{}

quickstreams::exception::BadTypeId::BadTypeId() {}
quickstreams::exception::BadTypeId::BadTypeId(
	const QString& msg
) :
	Exception(msg)
{}

quickstreams::exception::BadCast::BadCast() {}
quickstreams::exception::BadCast::BadCast(
	const QString& msg
) :
	Exception(msg)
{}

quickstreams::exception::BadWeakPtr::BadWeakPtr() {}
quickstreams::exception::BadWeakPtr::BadWeakPtr(
	const QString& msg
) :
	Exception(msg)
{}

quickstreams::exception::BadFunctionCall::BadFunctionCall() {}
quickstreams::exception::BadFunctionCall::BadFunctionCall(
	const QString& msg
) :
	Exception(msg)
{}

quickstreams::exception::BadAlloc::BadAlloc() {}
quickstreams::exception::BadAlloc::BadAlloc(
	const QString& msg
) :
	Exception(msg)
{}

quickstreams::exception::BadArrayNewLength::BadArrayNewLength() {}
quickstreams::exception::BadArrayNewLength::BadArrayNewLength(
	const QString& msg
) :
	BadAlloc(msg)
{}

quickstreams::exception::BadException::BadException() {}
quickstreams::exception::BadException::BadException(
	const QString& msg
) :
	Exception(msg)
{}

QString quickstreams::exception::JsException::name() const {
	return _name;
}

QVariant quickstreams::exception::JsException::data() const {
	return _data;
}

quickstreams::exception::LogicError::LogicError() {}
quickstreams::exception::LogicError::LogicError(const QString &msg) :
	Exception(msg)
{}

quickstreams::exception::InvalidArgument::InvalidArgument() {}
quickstreams::exception::InvalidArgument::InvalidArgument(const QString &msg) :
	LogicError(msg)
{}

quickstreams::exception::DomainError::DomainError() {}
quickstreams::exception::DomainError::DomainError(const QString &msg) :
	LogicError(msg)
{}

quickstreams::exception::LengthError::LengthError() {}
quickstreams::exception::LengthError::LengthError(const QString &msg) :
	LogicError(msg)
{}

quickstreams::exception::OutOfRange::OutOfRange() {}
quickstreams::exception::OutOfRange::OutOfRange(const QString &msg) :
	LogicError(msg)
{}

quickstreams::exception::FutureError::FutureError() {}
quickstreams::exception::FutureError::FutureError(std::error_code code) :
	LogicError(),
	_code(code)
{}

std::error_code quickstreams::exception::FutureError::code() const {
	return _code;
}

quickstreams::exception::RuntimeError::RuntimeError() {}
quickstreams::exception::RuntimeError::RuntimeError(const QString &msg) :
	Exception(msg)
{}

quickstreams::exception::RangeError::RangeError() {}
quickstreams::exception::RangeError::RangeError(const QString &msg) :
	RuntimeError(msg)
{}

quickstreams::exception::OverflowError::OverflowError() {}
quickstreams::exception::OverflowError::OverflowError(const QString &msg) :
	RuntimeError(msg)
{}

quickstreams::exception::UnderflowError::UnderflowError() {}
quickstreams::exception::UnderflowError::UnderflowError(const QString &msg) :
	RuntimeError(msg)
{}

quickstreams::exception::RegexError::RegexError() {}
quickstreams::exception::RegexError::RegexError(
	std::regex_constants::error_type regexErrorType
) :
	RuntimeError(),
	_code(regexErrorType)
{}

std::regex_constants::error_type
quickstreams::exception::RegexError::code() const {
	return _code;
}

quickstreams::exception::SystemError::SystemError() {}
quickstreams::exception::SystemError::SystemError(
	const QString &msg,
	std::error_code code
) :
	RuntimeError(msg),
	_code(code)
{}

std::error_code quickstreams::exception::SystemError::code() const {
	return _code;
}

// Types
int quickstreams::exception::Exception::type() {
	return qMetaTypeId<quickstreams::exception::Exception*>();
}

int quickstreams::exception::JsException::type() {
	return qMetaTypeId<quickstreams::exception::JsException*>();
}

int quickstreams::exception::BadTypeId::type() {
	return qMetaTypeId<quickstreams::exception::BadTypeId*>();
}

int quickstreams::exception::BadCast::type() {
	return qMetaTypeId<quickstreams::exception::BadCast*>();
}

int quickstreams::exception::BadWeakPtr::type() {
	return qMetaTypeId<quickstreams::exception::BadWeakPtr*>();
}

int quickstreams::exception::BadFunctionCall::type() {
	return qMetaTypeId<quickstreams::exception::BadFunctionCall*>();
}

int quickstreams::exception::BadAlloc::type() {
	return qMetaTypeId<quickstreams::exception::BadAlloc*>();
}

int quickstreams::exception::BadArrayNewLength::type() {
	return qMetaTypeId<quickstreams::exception::BadArrayNewLength*>();
}

int quickstreams::exception::BadException::type() {
	return qMetaTypeId<quickstreams::exception::BadException*>();
}

int quickstreams::exception::LogicError::type() {
	return qMetaTypeId<quickstreams::exception::LogicError*>();
}

int quickstreams::exception::InvalidArgument::type() {
	return qMetaTypeId<quickstreams::exception::InvalidArgument*>();
}

int quickstreams::exception::DomainError::type() {
	return qMetaTypeId<quickstreams::exception::DomainError*>();
}

int quickstreams::exception::LengthError::type() {
	return qMetaTypeId<quickstreams::exception::LengthError*>();
}

int quickstreams::exception::OutOfRange::type() {
	return qMetaTypeId<quickstreams::exception::OutOfRange*>();
}

int quickstreams::exception::FutureError::type() {
	return qMetaTypeId<quickstreams::exception::FutureError*>();
}

int quickstreams::exception::RuntimeError::type() {
	return qMetaTypeId<quickstreams::exception::RuntimeError*>();
}

int quickstreams::exception::RangeError::type() {
	return qMetaTypeId<quickstreams::exception::RangeError*>();
}

int quickstreams::exception::OverflowError::type() {
	return qMetaTypeId<quickstreams::exception::OverflowError*>();
}

int quickstreams::exception::UnderflowError::type() {
	return qMetaTypeId<quickstreams::exception::UnderflowError*>();
}

int quickstreams::exception::RegexError::type() {
	return qMetaTypeId<quickstreams::exception::RegexError*>();
}

int quickstreams::exception::SystemError::type() {
	return qMetaTypeId<quickstreams::exception::SystemError*>();
}

quickstreams::Error::Error(exception::Exception* instance) {
	if(!instance) return;
	_obj = Reference(instance, &exception::Exception::deleteLater);
}

bool quickstreams::Error::isNull() const {
	return _obj.isNull();
}

const quickstreams::exception::Exception*
quickstreams::Error::operator->() {
	// If the error doesn't reference any exception then create a default one
	// to avoid occasional read access violations during runtime
	if(_obj.isNull()) _obj = Reference(
		new exception::Exception(), &exception::Exception::deleteLater
	);
	return _obj.data();
}

int quickstreams::Error::type() const {
	if(_obj.isNull()) return QMetaType::Void;
	auto meta(_obj->metaObject());
	if(!meta) return QMetaType::Void;
	QString name(meta->className());
	name += '*';
	return QMetaType::type(name.toLatin1());
}

bool quickstreams::Error::is(int type) const {
	return type == this->type();
}

bool quickstreams::Error::is(const QString& name) const {
	if(!is(quickstreams::exception::JsException::type())) return false;
	return static_cast<quickstreams::exception::JsException*>(
		_obj.data()
	)->name() == name;
}

bool quickstreams::Error::is(const QJSValue& type) const {
	if(type.isNumber()) return is(type.toInt());
	else if(type.isString()) return is(type.toString());
	else return false;
}

QString quickstreams::Error::name() const {
	if(!is(quickstreams::exception::JsException::type())) return "";
	return static_cast<quickstreams::exception::JsException*>(
		_obj.data()
	)->name();
}

QString quickstreams::Error::message() const {
	if(_obj.isNull()) return "";
	return _obj->message();
}

static void __register_quickstreams_qml_error_types() {
    qRegisterMetaType<quickstreams::exception::Exception*>();
	qRegisterMetaType<quickstreams::exception::JsException*>();
	qRegisterMetaType<quickstreams::exception::BadTypeId*>();
	qRegisterMetaType<quickstreams::exception::BadCast*>();
	qRegisterMetaType<quickstreams::exception::BadWeakPtr*>();
	qRegisterMetaType<quickstreams::exception::BadFunctionCall*>();
	qRegisterMetaType<quickstreams::exception::BadAlloc*>();
	qRegisterMetaType<quickstreams::exception::BadArrayNewLength*>();
	qRegisterMetaType<quickstreams::exception::BadException*>();
	qRegisterMetaType<quickstreams::exception::LogicError*>();
    qRegisterMetaType<quickstreams::exception::InvalidArgument*>();
    qRegisterMetaType<quickstreams::exception::DomainError*>();
    qRegisterMetaType<quickstreams::exception::LengthError*>();
    qRegisterMetaType<quickstreams::exception::OutOfRange*>();
    qRegisterMetaType<quickstreams::exception::FutureError*>();
    qRegisterMetaType<quickstreams::exception::RuntimeError*>();
	qRegisterMetaType<quickstreams::exception::RangeError*>();
	qRegisterMetaType<quickstreams::exception::OverflowError*>();
	qRegisterMetaType<quickstreams::exception::UnderflowError*>();
	qRegisterMetaType<quickstreams::exception::RegexError*>();
	qRegisterMetaType<quickstreams::exception::SystemError*>();
}

Q_COREAPP_STARTUP_FUNCTION(__register_quickstreams_qml_error_types)
