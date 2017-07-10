#pragma once

#include <exception>
#include <future>
#include <regex>
#include <QObject>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QJSValue>
#include <QSharedPointer>

namespace quickstreams {
namespace exception {

class Exception : public QObject {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

protected:
	QString _msg;

public:
	static int type();

	Exception();
	Exception(const QString& msg);
	QString message() const;
};

// QML exceptions
class JsException : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)
	Q_PROPERTY(QString nane READ name CONSTANT)
	Q_PROPERTY(QVariant data READ data CONSTANT)

protected:
	QString _name;
	QVariant _data;

public:
	static int type();

	JsException();
	JsException(
		const QString& name,
		const QString& msg,
		const QVariant& data
	);
	QString name() const;
	QVariant data() const;
};

// std::bad_typeid replica
class BadTypeId : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadTypeId();
	BadTypeId(const QString& msg);
};

// std::bad_cast replica
class BadCast : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadCast();
	BadCast(const QString& msg);
};

// std::bad_weak_ptr replica
class BadWeakPtr : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadWeakPtr();
	BadWeakPtr(const QString& msg);
};

// std::bad_function_call replica
class BadFunctionCall : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadFunctionCall();
	BadFunctionCall(const QString& msg);
};

// std::bad_alloc replica
class BadAlloc : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadAlloc();
	BadAlloc(const QString& msg);
};

// std::bad_array_new_length replica
class BadArrayNewLength : public BadAlloc {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadArrayNewLength();
	BadArrayNewLength(const QString& msg);
};

// std::bad_exception replica
class BadException : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	BadException();
	BadException(const QString& msg);
};

// std::logic_error replica
class LogicError : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	LogicError();
	LogicError(const QString& msg);
};

// std::invalid_argument replica
class InvalidArgument : public LogicError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	InvalidArgument();
	InvalidArgument(const QString& msg);
};

// std::domain_error replica
class DomainError : public LogicError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	DomainError();
	DomainError(const QString& msg);
};

// std::length_error replica
class LengthError : public LogicError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	LengthError();
	LengthError(const QString& msg);
};

// std::out_of_range replica
class OutOfRange : public LogicError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	OutOfRange();
	OutOfRange(const QString& msg);
};

// std::future_error replica
class FutureError : public LogicError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

protected:
	std::error_code _code;

public:
	static int type();

	FutureError();
	FutureError(std::error_code code);

	std::error_code code() const;
};

// std::runtime_error replica
class RuntimeError : public Exception {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	RuntimeError();
	RuntimeError(const QString& msg);
};

// std::range_error replica
class RangeError : public RuntimeError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	RangeError();
	RangeError(const QString& msg);
};

// std::overflow_error replica
class OverflowError : public RuntimeError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	OverflowError();
	OverflowError(const QString& msg);
};

// std::underflow_error replica
class UnderflowError : public RuntimeError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	static int type();

	UnderflowError();
	UnderflowError(const QString& msg);
};

// std::regex_error replica
class RegexError : public RuntimeError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

protected:
	std::regex_constants::error_type _code;

public:
	static int type();

	RegexError();
	RegexError(std::regex_constants::error_type regexErrorType);

	std::regex_constants::error_type code() const;
};

// std::system_error replica
class SystemError : public RuntimeError {
	Q_OBJECT
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

protected:
	std::error_code _code;

public:
	static int type();

	SystemError();
	SystemError(const QString& msg, std::error_code code);

	std::error_code code() const;
};

}} // quickstreams::exception

namespace quickstreams {

class Error {
	friend class Executable;

	Q_GADGET
	Q_PROPERTY(int type READ type CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString message READ message CONSTANT)

public:
	typedef QSharedPointer<exception::Exception> Reference;

private:
	Reference _obj;

public:
	Error(exception::Exception* instance = nullptr);
	bool isNull() const;
	const exception::Exception* operator->();
	int type() const;
	bool is(int type) const;
	bool is(const QString& name) const;
	Q_INVOKABLE bool is(const QJSValue& type) const;
	QString name() const;
	QString message() const;

	template <typename T>
	T* as() {
		// If the error doesn't reference any exception - create a default one
		// to avoid occasional read access violations during runtime
		if(_obj.isNull()) _obj = Reference(
			new exception::Exception(), &exception::Exception::deleteLater
		);
		return static_cast<T*>(_obj.data());
	}
};

}

Q_DECLARE_METATYPE(quickstreams::exception::Exception*)
Q_DECLARE_METATYPE(quickstreams::exception::JsException*)
Q_DECLARE_METATYPE(quickstreams::exception::BadTypeId*)
Q_DECLARE_METATYPE(quickstreams::exception::BadCast*)
Q_DECLARE_METATYPE(quickstreams::exception::BadWeakPtr*)
Q_DECLARE_METATYPE(quickstreams::exception::BadFunctionCall*)
Q_DECLARE_METATYPE(quickstreams::exception::BadAlloc*)
Q_DECLARE_METATYPE(quickstreams::exception::BadArrayNewLength*)
Q_DECLARE_METATYPE(quickstreams::exception::BadException*)
Q_DECLARE_METATYPE(quickstreams::exception::LogicError*)
Q_DECLARE_METATYPE(quickstreams::exception::InvalidArgument*)
Q_DECLARE_METATYPE(quickstreams::exception::DomainError*)
Q_DECLARE_METATYPE(quickstreams::exception::LengthError*)
Q_DECLARE_METATYPE(quickstreams::exception::OutOfRange*)
Q_DECLARE_METATYPE(quickstreams::exception::FutureError*)
Q_DECLARE_METATYPE(quickstreams::exception::RuntimeError*)
Q_DECLARE_METATYPE(quickstreams::exception::RangeError*)
Q_DECLARE_METATYPE(quickstreams::exception::OverflowError*)
Q_DECLARE_METATYPE(quickstreams::exception::UnderflowError*)
Q_DECLARE_METATYPE(quickstreams::exception::RegexError*)
Q_DECLARE_METATYPE(quickstreams::exception::SystemError*)
Q_DECLARE_METATYPE(quickstreams::Error)
