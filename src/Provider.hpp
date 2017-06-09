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

	Stream::Reference internalCreate(
		const Executable::Reference& executable,
		Stream::Type type = Stream::Type::Atomic
	);

	void registerNew(const Stream::Reference& reference);
	void dispose(Stream* stream);
	Stream::Reference reference(Stream* stream) const;

public:
	explicit Provider(QObject* parent = nullptr);
	Stream::Reference create(
		LambdaExecutable::Function function,
		Stream::Type type = Stream::Type::Atomic
	);
};

} // quickstreams
