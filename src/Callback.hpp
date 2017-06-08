#pragma once

#include <QVariant>
#include <QSharedPointer>

namespace quickstreams {

class Callback {
public:
	typedef QSharedPointer<Callback> Reference;

public:
	virtual ~Callback() {}
	virtual void execute(const QVariant& data) = 0;
};

} // quickstreams
