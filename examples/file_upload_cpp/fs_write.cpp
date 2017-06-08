#include "Filesystem.hpp"
#include <QuickStreams>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QDebug>

using namespace quickstreams;

Stream::Reference Filesystem::write(
	const QString& fileId,
	int position,
	int length
) {
	qDebug() << "FS::write > write to file: " << fileId
		<< " at byte " << position
		<< " for " << length << "bytes";

	// Create and return an atomic stream representing the write operating
	return _streamProvider->create([this, fileId, position, length](
		const StreamHandle& mainStream,
		const QVariant& data
	) {
		Q_UNUSED(data)
		// Make the main returned stream adopt a subordinary stream
		// the awakening of which is delayed for a random amount of time.
		// The subordinate stream closes the mainStream when finally awoken
		mainStream.adopt(_streamProvider->create([
			mainStream, fileId, position, length
		](
			const StreamHandle& stream,
			const QVariant& data
		) {
			Q_UNUSED(stream)
			Q_UNUSED(data)
			qDebug() << "FS::write > write to " << fileId
				<< " (" << position << ":" << length << ") completed";
			mainStream.close();
			return nullptr;
		}))
		->delay(randomLatency());
		return nullptr;
	});
}
