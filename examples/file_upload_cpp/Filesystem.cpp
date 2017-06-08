#include "Filesystem.hpp"
#include <QuickStreams>
#include <QObject>
#include <QString>

using namespace quickstreams;
using namespace quickstreams::qml;

Filesystem::Filesystem(
	Provider* streamProvider,
	int minLatency,
	int maxLatency
) :
	QObject(nullptr),
	_streamProvider(streamProvider),
	_minLatency(minLatency),
	_maxLatency(maxLatency)
{

}

QString Filesystem::randomId(int length) {
	const QString possibleCharacters("ABCDEF0123456789");
	QString randomString;
	for(int itr(0); itr < length; ++itr) {
		int index(qrand() % possibleCharacters.length());
		auto nextChar(possibleCharacters.at(index));
		randomString.append(nextChar);
	}
	return randomString;
}

int Filesystem::randomLatency() const {
	return qrand() % (_maxLatency - _minLatency + 1) + _minLatency;
}
