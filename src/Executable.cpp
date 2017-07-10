#include "Executable.hpp"
#include "StreamHandle.hpp"
#include "Error.hpp"
#include <QVariant>

quickstreams::Executable::Executable() :
	_handle(nullptr),
	_returnedStream(nullptr)
{}

void quickstreams::Executable::setHandle(StreamHandle* handle) {
	_handle = handle;
}

void quickstreams::Executable::reset() {
	_error._obj.reset();
	_returnedStream = nullptr;
}

bool quickstreams::Executable::hasFailed() const {
	return !_error._obj.isNull();
}

bool quickstreams::Executable::hasReturnedStream() const {
	return _returnedStream != nullptr;
}

QVariant quickstreams::Executable::getError() const {
	return QVariant::fromValue<Error>(_error);
}

quickstreams::Stream* quickstreams::Executable::stream() {
	return _returnedStream;
}
