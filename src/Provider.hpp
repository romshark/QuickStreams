#pragma once

#include "ProviderInterface.hpp"
#include "Stream.hpp"
#include "Executable.hpp"
#include "LambdaExecutable.hpp"
#include <QObject>
#include <QHash>

#include <QString>

namespace quickstreams {

namespace qml {
	class QmlProvider;
	class StreamConversion;
}

class Provider : public QObject, public ProviderInterface {
	Q_OBJECT
	friend class qml::QmlProvider;
	friend class qml::StreamConversion;

protected:
	typedef QHash<Stream*, Stream::Reference> ReferenceMap;

protected:
	ReferenceMap _references;
	quint64 _totalCreated;
	quint64 _totalExisting;
	quint64 _totalActive;

	Stream::Reference internalCreate(
		const Executable::Reference& executable,
		Stream::Type type = Stream::Type::Atomic
	);

	void registerNew(const Stream::Reference& reference);
	void activated();
	void finished();
	void destroyed();
	void dispose(Stream* stream);
	Stream::Reference reference(Stream* stream) const;

public:
	explicit Provider(QObject* parent = nullptr);
	Stream::Reference create(
		LambdaExecutable::Function function,
		Stream::Type type = Stream::Type::Atomic
	);

	quint64 totalCreated() const;
	quint64 totalExisting() const;
	quint64 totalActive() const;

signals:
	void totalCreatedChanged();
	void totalExistingChanged();
	void totalActiveChanged();
};

} // quickstreams
