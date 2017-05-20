#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

streams::StreamHandle::StreamHandle() {}

streams::StreamHandle::StreamHandle(
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

void streams::StreamHandle::event(const QString& name, const QVariant& data) {
	_eventCb(name, data);
}

void streams::StreamHandle::close(const QVariant& data) {
	if(_awaiting) return;
	_closeCb(data);
}

void streams::StreamHandle::fail(const QVariant& data) {
	if(_awaiting) return;
	_failCb(data);
}

void streams::StreamHandle::await(const QVariant& data) {
	if(_awaiting) return;
	if(_awaitCb(data)) {
		_awaiting = true;
	}
}

bool streams::StreamHandle::isAwaiting() const {
	return _awaiting;
}

Q_DECLARE_METATYPE(streams::StreamHandle)
