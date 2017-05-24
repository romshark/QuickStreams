#include "StreamHandle.hpp"
#include <QVariant>
#include <QString>

quickstreams::StreamHandle::StreamHandle() {}

quickstreams::StreamHandle::StreamHandle(
	EventCallback eventCb,
	CloseCallback closeCb,
	FailCallback failCb,
	WrapCallback wrapCb,
	AdoptCallback adoptCb,
	ReferenceGetter refGet,
	IsAbortableCallback isAbortableCb,
	IsAbortedCallback isAbortedCb
) :
	_isWrapper(false),
	_eventCb(eventCb),
	_closeCb(closeCb),
	_failCb(failCb),
	_wrapCb(wrapCb),
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
	if(_isWrapper) return;
	_closeCb(data);
}

void quickstreams::StreamHandle::fail(const QVariant& data) {
	if(_isWrapper) return;
	_failCb(data);
}

void quickstreams::StreamHandle::wrap(const QVariant& data) {
	if(_isWrapper) return;
	if(_wrapCb(data)) {
		_isWrapper = true;
	}
}

quickstreams::Stream* quickstreams::StreamHandle::adopt(
	const QVariant& target
) {
	if(_isWrapper) return nullptr;
	return _adoptCb(target);
}

bool quickstreams::StreamHandle::isWrapper() const {
	return _isWrapper;
}

bool quickstreams::StreamHandle::isAbortable() const {
	return _isAbortableCb();
}

bool quickstreams::StreamHandle::isAborted() const {
	return _isAbortedCb();
}

Q_DECLARE_METATYPE(quickstreams::StreamHandle)
