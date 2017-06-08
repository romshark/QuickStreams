#pragma once

#include <QVariant>
#include <QSharedPointer>

namespace quickstreams {

class Repeater {
public:
	typedef QSharedPointer<Repeater> Reference;

public:
	virtual ~Repeater() {}
	virtual bool evaluate(bool isAborted) = 0;
};

} // quickstreams
