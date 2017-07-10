#pragma once

#include "Retryer.hpp"
#include "JsTypeRetryer.hpp"
#include <QVariant>
#include <QString>
#include <QSet>

namespace quickstreams {

class JsTypeRetryer : public Retryer {
public:
	typedef QSet<int> Identifiers;
	typedef QSet<QString> Names;

protected:
	Identifiers _ids;
	Names _names;

public:
	JsTypeRetryer(const QVariant& errorTypes, qint32 maxTrials);
	bool verifyCondition(const QVariant& error);
};

} // quickstreams
