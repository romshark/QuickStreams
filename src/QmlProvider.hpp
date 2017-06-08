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
};

}} // quickstreams::qml
