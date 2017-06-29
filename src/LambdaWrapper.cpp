#include "LambdaWrapper.hpp"
#include <QVariant>
#include <exception>
#include <QSharedPointer>

quickstreams::LambdaWrapper::LambdaWrapper(Function function) :
	_function(function)
{}

void quickstreams::LambdaWrapper::execute(const QVariant& data) {
	// If the function is null then close the stream referencing this handle
	// because otherwise it would try to execute it causing a segfault
	if(!_function) {
		_handle->close(data);
		return;
	}

	// Try to execute
	try {
		StreamReference stream(_function(data));

		// If the function didn't return any stream then the stream
		// referencing this handle should close immediately as it would
		// otherwise never be closed because the stream handle is unavailable.
		if(stream.isNull()) {
			_handle->close(data);
			return;
		}
		_returnedStream = stream.data();
	} catch(const std::exception& error) {
		_error.reset(new QVariant(error.what()));
	} catch(...) {
		_error.reset(new QVariant());
	}
}
