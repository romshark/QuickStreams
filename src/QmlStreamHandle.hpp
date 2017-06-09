#pragma once

#include "StreamHandle.hpp"
#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>

namespace quickstreams {
namespace qml {

class QmlStream;

class QmlStreamHandle {
	friend class QmlStream;
	Q_GADGET
	Q_PROPERTY(bool isAbortable READ isAbortable)
	Q_PROPERTY(bool isAborted READ isAborted)

public:
	typedef std::function<QmlStream*(QmlStream*)> AdoptCallback;
	typedef std::function<QmlStream*()> ReferenceGetter;

protected:
	quickstreams::StreamHandle* _handle;
	AdoptCallback _adoptCb;
	ReferenceGetter _refGet;

	QmlStream* reference() const;

	QmlStreamHandle(
		quickstreams::StreamHandle* handle,
		AdoptCallback adoptCb,
		ReferenceGetter refGet
	);

public:
	QmlStreamHandle();

	// Makes this stream immediately emit an event optionally passing any data.
	Q_INVOKABLE void event(
		const QVariant& name,
		const QVariant& data = QVariant()
	) const;

	// Immediately closes this stream optionally passing any data.
	Q_INVOKABLE void close(const QVariant& data = QVariant()) const;

	// Immediately fails the stream optionally passing any data
	// describing the reason of failure.
	Q_INVOKABLE void fail(const QVariant& data = QVariant()) const;

	// Adopts the passed stream making this stream become its parent
	// and returns the passed stream to allow for chaining
	Q_INVOKABLE QmlStream* adopt(QmlStream* stream) const;

	bool isAbortable() const;
	bool isAborted() const;
};

}} // quickstreams::qml

Q_DECLARE_METATYPE(quickstreams::qml::QmlStreamHandle)
