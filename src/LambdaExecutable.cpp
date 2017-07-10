#include "LambdaExecutable.hpp"
#include "Error.hpp"
#include <QVariant>
#include <exception>
#include <QSharedPointer>

quickstreams::LambdaExecutable::LambdaExecutable(Function function) :
	_function(function)
{}

void quickstreams::LambdaExecutable::execute(const QVariant& data) {
	// If the function is null then close the stream referencing this handle
	// because otherwise it would try to execute it causing a segfault
	if(!_function) {
		_handle->close(data);
		return;
	}

	// Try to execute
	try {
		// Execute and forward closure responsibility to the user
		_function(*_handle, data);
	} catch(const Error& error) {
		_error = error;
	} catch(const std::invalid_argument& error) {
		_error = Error(new exception::InvalidArgument(error.what()));
	} catch(const std::domain_error& error) {
		_error = Error(new exception::DomainError(error.what()));
	} catch(const std::length_error& error) {
		_error = Error(new exception::LengthError(error.what()));
	} catch(const std::out_of_range& error) {
		_error = Error(new exception::OutOfRange(error.what()));
	} catch(const std::future_error& error) {
		_error = Error(new exception::FutureError(error.code()));
	} catch(const std::logic_error& error) {
		_error = Error(new exception::LogicError(error.what()));
	} catch(const std::range_error& error) {
		_error = Error(new exception::RangeError(error.what()));
	} catch(const std::overflow_error& error) {
		_error = Error(new exception::OverflowError(error.what()));
	} catch(const std::underflow_error& error) {
		_error = Error(new exception::UnderflowError(error.what()));
	} catch(const std::regex_error& error) {
		_error = Error(new exception::RegexError(error.code()));
	} catch(const std::system_error& error) {
		_error = Error(new exception::SystemError(error.what(), error.code()));
	} catch(const std::runtime_error& error) {
		_error = Error(new exception::RuntimeError(error.what()));
	} catch(const std::bad_typeid& error) {
		_error = Error(new exception::BadTypeId(error.what()));
	} catch(const std::bad_cast& error) {
		_error = Error(new exception::BadCast(error.what()));
	} catch(const std::bad_weak_ptr& error) {
		_error = Error(new exception::BadWeakPtr(error.what()));
	} catch(const std::bad_function_call& error) {
		_error = Error(new exception::BadFunctionCall(error.what()));
	} catch(const std::bad_array_new_length& error) {
		_error = Error(new exception::BadArrayNewLength(error.what()));
	} catch(const std::bad_alloc& error) {
		_error = Error(new exception::BadAlloc(error.what()));
	} catch(const std::bad_exception& error) {
		_error = Error(new exception::BadException(error.what()));
	} catch(const std::exception& error) {
		_error = Error(new exception::Exception(error.what()));
	} catch(const char* error) {
		_error = Error(new exception::Exception(error));
	} catch(const std::string& error) {
		_error = Error(new exception::Exception(QString::fromStdString(error)));
	} catch(const QString& error) {
		_error = Error(new exception::Exception(error));
	} catch(...) {
		_error = Error(new exception::Exception("Unkown error"));
	}
}
