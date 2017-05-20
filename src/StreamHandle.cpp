#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

quickstreams::StreamHandle::StreamHandle() {}

quickstreams::StreamHandle::StreamHandle(
	EventCallback eventCb,
	CloseCallback closeCb,
	FailCallback failCb,
	AwaitCallback awaitCb
) :
	_awaiting(false),
	_eventCb(eventCb),
	_closeCb(closeCb),
	_failCb(failCb),
	_awaitCb(awaitCb)
{}

void quickstreams::StreamHandle::event(
	const QString& name,
	const QVariant& data
) {
	_eventCb(name, data);
}

void quickstreams::StreamHandle::close(const QVariant& data) {
	if(_awaiting) return;
	_closeCb(data);
}

void quickstreams::StreamHandle::fail(const QVariant& data) {
	if(_awaiting) return;
	_failCb(data);
}

void quickstreams::StreamHandle::await(const QVariant& data) {
	if(_awaiting) return;
	if(_awaitCb(data)) {
		_awaiting = true;
	}
}

bool quickstreams::StreamHandle::isAwaiting() const {
	return _awaiting;
}

Q_DECLARE_METATYPE(quickstreams::StreamHandle)
