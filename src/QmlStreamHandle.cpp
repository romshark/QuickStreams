#include "QmlStreamHandle.hpp"
#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

quickstreams::qml::QmlStreamHandle::QmlStreamHandle() :
	_handle(nullptr),
	_adoptCb(nullptr),
	_refGet(nullptr)
{}


quickstreams::qml::QmlStreamHandle::QmlStreamHandle(
	quickstreams::StreamHandle* handle,
	AdoptCallback adoptCb,
	ReferenceGetter refGet
) :
	_handle(handle),
	_adoptCb(adoptCb),
	_refGet(refGet)
{}

quickstreams::qml::QmlStream*
quickstreams::qml::QmlStreamHandle::reference() const {
	return _refGet();
}

void quickstreams::qml::QmlStreamHandle::event(
	const QVariant& name,
	const QVariant& data
) const {
	if(!name.canConvert<QString>()) return;
	_handle->_eventCb(name.toString(), data);
}

void quickstreams::qml::QmlStreamHandle::close(const QVariant& data) const {
	_handle->_closeCb(data);
}

void quickstreams::qml::QmlStreamHandle::fail(const QVariant& data) const {
	_handle->_failCb(data);
}

quickstreams::qml::QmlStream* quickstreams::qml::QmlStreamHandle::adopt(
	QmlStream* stream
) const {
	return _adoptCb(stream);
}

bool quickstreams::qml::QmlStreamHandle::isAbortable() const {
	return _handle->_isAbortableCb();
}

bool quickstreams::qml::QmlStreamHandle::isAborted() const {
	return _handle->_isAbortedCb();
}
