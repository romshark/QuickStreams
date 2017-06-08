#include "Filesystem.hpp"
#include <QuickStreams>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QDebug>

using namespace quickstreams;

quickstreams::Stream::Reference Filesystem::allocateFile() {
	qDebug() << "FS::allocateFile > allocate a new file";

	// Create and return an atomic stream representing the allocation operation
	return _streamProvider->create([this](
		const StreamHandle& stream,
		const QVariant& data
	) {
		Q_UNUSED(data)
		// Close the returned stream after a random amount of time
		QTimer::singleShot(randomLatency(), [stream]() {
			QString newId(Filesystem::randomId(8));
			qDebug() << "FS::allocateFile > file allocated: " << newId;
			stream.close(newId);
			return nullptr;
		});
		return nullptr;
	});
}
