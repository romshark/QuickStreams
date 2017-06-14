#include "LambdaExecutable.hpp"
#include <QVariant>
#include <exception>
#include <QSharedPointer>

quickstreams::LambdaExecutable::LambdaExecutable(Function function) :
	_function(function)
{}

void quickstreams::LambdaExecutable::execute(const QVariant& data) {
	if(!_function) return;
	try {
		// Execute
		QSharedPointer<Stream> stream(_function(*_handle, data));
		if(!stream.isNull()) _returnedStream = stream.data();
	} catch(const std::exception& error) {
		// Execution failed
		//TODO: turn error into variant
		_error.reset(new QVariant(error.what()));
	} catch(...) {
		_error.reset(new QVariant());
	}
}
