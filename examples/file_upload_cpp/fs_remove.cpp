#include "Filesystem.hpp"
#include <QuickStreams>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QDebug>

using namespace quickstreams;

Stream::Reference Filesystem::remove(const QString& fileId) {
	qDebug() << "FS::remove > removing file: " << fileId;

	// Create and return an atomic stream representing the removal operation
	return _streamProvider->create([this, fileId](
		const StreamHandle& stream,
		const QVariant& data
	) {
		Q_UNUSED(data)
		// Close the returned stream after a random amount of time
		QTimer::singleShot(randomLatency(), [stream, fileId]() {
			qDebug() << "FS::remove > file " << fileId << " has been removed";
			stream.close();
			return nullptr;
		});
		return nullptr;
	});
}
