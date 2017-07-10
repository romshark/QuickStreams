#pragma once

#include "Retryer.hpp"
#include "TypeRetryer.hpp"
#include <QVariant>
#include <QVector>

namespace quickstreams {

class TypeRetryer : public Retryer {
public:
	typedef QVector<int> TypeList;

protected:
	TypeList _errorTypes;

public:
	TypeRetryer(const TypeList& errorTypes, qint32 maxTrials);
	bool verifyCondition(const QVariant& error);
};

} // quickstreams
