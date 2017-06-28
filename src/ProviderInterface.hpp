#pragma once

#include <QSharedPointer>

namespace quickstreams {

class Stream;

class ProviderInterface {
public:
	~ProviderInterface() {}

	virtual void dispose(Stream* stream) = 0;
	virtual void activated() = 0;
	virtual void finished() = 0;
	virtual void destroyed() = 0;

	virtual void registerNew(const QSharedPointer<Stream>& stream) = 0;
	virtual QSharedPointer<Stream> reference(Stream* stream) const = 0;

	virtual quint64 totalCreated() const = 0;
	virtual quint64 totalExisting() const = 0;
	virtual quint64 totalActive() const = 0;
};

}
