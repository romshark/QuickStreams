#include "LambdaSyncExecutable.hpp"
#include <QVariant>
#include <exception>
#include <QSharedPointer>

quickstreams::LambdaSyncExecutable::LambdaSyncExecutable(Function function) :
	_function(function)
{}

void quickstreams::LambdaSyncExecutable::execute(const QVariant& data) {
	// If the function is null then close the stream referencing this handle
	// because otherwise it would try to execute it causing a segfault
	if(!_function) {
		_handle->close(data);
		return;
	}

	// Try to execute
	try {
		// Execute and keep closure responsibility by the executable
		_handle->close(_function(data));
	} catch(const std::exception& error) {
		_error.reset(new QVariant(error.what()));
	} catch(...) {
		_error.reset(new QVariant());
	}
}
