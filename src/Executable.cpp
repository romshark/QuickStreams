#include "Executable.hpp"
#include "StreamHandle.hpp"
#include <QVariant>

quickstreams::Executable::Executable() :
	_handle(nullptr),
	_error(nullptr),
	_returnedStream(nullptr)
{}

void quickstreams::Executable::setHandle(StreamHandle* handle) {
	_handle = handle;
}

void quickstreams::Executable::reset() {
	_error.reset();
	_returnedStream = nullptr;
}

bool quickstreams::Executable::hasFailed() const {
	return _error != nullptr;
}

bool quickstreams::Executable::hasReturnedStream() const {
	return _returnedStream != nullptr;
}

QVariant quickstreams::Executable::getError() const {
	if(_error.isNull()) return QVariant();
	return *_error;
}

quickstreams::Stream* quickstreams::Executable::stream() {
	return _returnedStream;
}
