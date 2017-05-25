#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

quickstreams::StreamHandle::StreamHandle() {}

quickstreams::StreamHandle::StreamHandle(
	EventCallback eventCb,
	CloseCallback closeCb,
	FailCallback failCb,
	AdoptCallback adoptCb,
	ReferenceGetter refGet,
	IsAbortableCallback isAbortableCb,
	IsAbortedCallback isAbortedCb
) :
	_eventCb(eventCb),
	_closeCb(closeCb),
	_failCb(failCb),
	_adoptCb(adoptCb),
	_refGet(refGet),
	_isAbortableCb(isAbortableCb),
	_isAbortedCb(isAbortedCb)
{}

quickstreams::Stream* quickstreams::StreamHandle::reference() const {
	return _refGet();
}

void quickstreams::StreamHandle::event(
	const QString& name,
	const QVariant& data
) {
	_eventCb(name, data);
}

void quickstreams::StreamHandle::close(const QVariant& data) {
	_closeCb(data);
}

void quickstreams::StreamHandle::fail(const QVariant& data) {
	_failCb(data);
}

quickstreams::Stream* quickstreams::StreamHandle::adopt(
	const QVariant& target
) {
	return _adoptCb(target);
}

bool quickstreams::StreamHandle::isAbortable() const {
	return _isAbortableCb();
}

bool quickstreams::StreamHandle::isAborted() const {
	return _isAbortedCb();
}

Q_DECLARE_METATYPE(quickstreams::StreamHandle)
