#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

quickstreams::StreamHandle::StreamHandle(
	EventCallback eventCb,
	CloseCallback closeCb,
	FailCallback failCb,
	AdoptCallback adoptCb,
	IsAbortableCallback isAbortableCb,
	IsAbortedCallback isAbortedCb
) :
	_eventCb(eventCb),
	_closeCb(closeCb),
	_failCb(failCb),
	_adoptCb(adoptCb),
	_isAbortableCb(isAbortableCb),
	_isAbortedCb(isAbortedCb)
{}

quickstreams::StreamHandle::StreamHandle() {}

void quickstreams::StreamHandle::event(
	const QString& name,
	const QVariant& data
) const {
	_eventCb(name, data);
}

void quickstreams::StreamHandle::close(const QVariant& data) const {
	_closeCb(data);
}

void quickstreams::StreamHandle::fail(const QVariant& data) const {
	_failCb(data);
}

quickstreams::StreamHandle::StreamReference quickstreams::StreamHandle::adopt(
	StreamReference stream
) const {
	return _adoptCb(stream);
}

bool quickstreams::StreamHandle::isAbortable() const {
	return _isAbortableCb();
}

bool quickstreams::StreamHandle::isAborted() const {
	return _isAbortedCb();
}

Q_DECLARE_METATYPE(quickstreams::StreamHandle)
