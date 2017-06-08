#pragma once

#include <QSharedPointer>

namespace quickstreams {

class Stream;

class ProviderInterface {
public:
	~ProviderInterface() {}

	virtual void dispose(Stream* stream) = 0;
	virtual void registerNew(const QSharedPointer<Stream>& stream) = 0;
	virtual QSharedPointer<Stream> reference(Stream* stream) const = 0;
};

}
