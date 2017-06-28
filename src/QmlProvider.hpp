#pragma once

#include "Stream.hpp"
#include "QmlStream.hpp"
#include "Provider.hpp"
#include <QObject>
#include <QQmlEngine>
#include <QJSValue>

namespace quickstreams {
namespace qml {

class QmlProvider : public QObject {
	Q_OBJECT
	Q_PROPERTY(quickstreams::Stream::Type Atomic READ Atomic CONSTANT)
	Q_PROPERTY(quickstreams::Stream::Type Abortable READ Abortable CONSTANT)
	Q_PROPERTY(quint64 totalCreated READ totalCreated NOTIFY totalCreatedChanged)
	Q_PROPERTY(quint64 totalExisting READ totalExisting NOTIFY totalExistingChanged)
	Q_PROPERTY(quint64 totalActive READ totalActive NOTIFY totalActiveChanged)

protected:
	Provider* _provider;
	QQmlEngine* _engine;

public:
	explicit QmlProvider(QQmlEngine* engine, Provider* provider);

	QmlStream* toQml(const quickstreams::Stream::Reference& stream);

	Q_INVOKABLE QmlStream* create(
		const QJSValue& target,
		quickstreams::Stream::Type type = quickstreams::Stream::Type::Atomic
	);

	quickstreams::Stream::Type Atomic() const;
	quickstreams::Stream::Type Abortable() const;

	quint64 totalCreated() const;
	quint64 totalExisting() const;
	quint64 totalActive() const;

signals:
	void totalCreatedChanged();
	void totalExistingChanged();
	void totalActiveChanged();
};

}} // quickstreams::qml
