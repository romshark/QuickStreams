#include "Filesystem.hpp"
#include <QuickStreams>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVariantMap>
#include <QDebug>

using namespace quickstreams;

Stream::Reference Filesystem::uploadFile(int fileSize, int chunkSize) {

	// Declare mutable stream state structure
	struct UploadTransaction {
		// Milestones for eventual cleanup
		bool allocated;

		// Context variables for progress tracking
		int uploadProgress;
		QString fileId;

		UploadTransaction() :
			allocated(false),
			uploadProgress(0)
		{}
	};

	qDebug() << "FS::uploadFile > begin new upload";

	// Create a new, abortable, event emitting stream
	// to abstract away a complex, underlying stream sequence
	return _streamProvider->create([this, fileSize, chunkSize](
		const StreamHandle& mainStream,
		const QVariant& data
	) {
		Q_UNUSED(data)

		// The transaction state should be held inside a smart pointer
		// for it to be passed to asynchronous lambda functions
		QSharedPointer<UploadTransaction> transaction(new UploadTransaction);

		// mainStream is the parent of the following stream chain.
		// This setting is important for proper abortion handling.
		// When mainStream is requested to abort - this underlying chain
		// should properly abort execution and clean up if necessary

		// Prepare upload by allocating the file on the filesystem.
		// This operation will return an atomic stream on top of which
		// the following chain is build
		auto sequence(
			mainStream.adopt(allocateFile())

			// Indefinitely retry allocation in case of timeout errors
			->retry({(int)Error::TimeoutError})

			// Bind this asynchronous operation, it should be executed
			// no matter whether the chain was aborted in the meantime or not.
			// It will ensure that the file allocation milestone is marked
			// as achieved even if the chain was aborted while the filesystem
			// was allocating the file for proper clean up
			->bind([mainStream, transaction](
				const StreamHandle& stream, const QVariant& newFileId
			) {
				transaction->allocated = true;
				transaction->fileId = newFileId.toString();
				mainStream.event("allocated", QVariantMap({
					{"id", transaction->fileId}
				}));
				stream.close();
				return nullptr;
			})

			// Begin uploading the file chunk after chunk
			// when the file has successfuly been allocated
			->attach([this, transaction, chunkSize](
				const StreamHandle& stream,
				const QVariant& data
			) {
				Q_UNUSED(stream)
				Q_UNUSED(data)
				return write(
					transaction->fileId,
					transaction->uploadProgress,
					chunkSize
				);
			})

			// Indefinitely retry writing chunks in case of timeout errors
			->retry({(int)Error::TimeoutError})

			// Repeat writing until all chunks are written.
			// Write operations are atomic streams and will repeat even
			// if the file upload was aborted. To prevent the atomic stream
			// from repeating after abortion the "aborted" argument must be
			// checked manually to stop repetition in case of upload abortion
			->repeat([mainStream, transaction, fileSize, chunkSize] (
				bool aborted
			) {
				// update upload progress and emit 'chunk_uploaded' event
				// after each successful write
				transaction->uploadProgress += chunkSize;
				mainStream.event("chunk_uploaded", QVariantMap({
					{"progress", transaction->uploadProgress}
				}));

				// If upload was aborted then stop repeating write
				if(aborted) return false;

				// Continue repeating if the file isn't yet fully uploaded
				if(transaction->uploadProgress < fileSize) return true;

				// Otherwise stop writing
				return false;
			})
			// Close main stream when all chunks have successfuly been written
			->attach([mainStream, transaction](
				const StreamHandle& stream,
				const QVariant& data
			) {
				Q_UNUSED(data)
				mainStream.close(transaction->fileId);
				stream.close();
				return nullptr;
			})
		);

		// Declare a clean up sequence that is started in case
		// the file upload transaction must be rolled back
		auto cleanupSequence([this, mainStream, transaction](
			const QVariant& error
		) {
			if(!transaction->allocated) {
				if(!error.isNull()) {
					mainStream.fail(error);
					return;
				}
				mainStream.close();
				return;
			}

			// Try to remove the unfinished file
			remove(transaction->fileId)
			->attach([mainStream](
				const StreamHandle& stream,
				const QVariant& data
			) {
				Q_UNUSED(data)
				mainStream.close();
				stream.close();
				return nullptr;
			})
			// Catch unexpected clean up errors
			// and fail the main stream in such a case
			->failure([mainStream](
				const StreamHandle& stream,
				const QVariant& error
			) {
				mainStream.fail(error);
				stream.close();
				return nullptr;
			});
		});

		// Catch unexpected errors during chain execution
		// and clean up before failing the main stream
		sequence->failure([mainStream, cleanupSequence](
			const StreamHandle& stream,
			const QVariant& error
		) {
			qDebug() << "FS::uploadFile > catch failed file upload";
			mainStream.event("cleanup_after_error", error);
			cleanupSequence(error);
			stream.close();
			return nullptr;
		});

		// Clean up before abort-closing the main stream
		// if the upload chain was aborted
		sequence->abortion([mainStream, cleanupSequence](
			const StreamHandle& stream,
			const QVariant& error
		) {
			qDebug() << "FS::uploadFile > catch aborted file upload";
			mainStream.event("cleanup_after_abortion");
			cleanupSequence(error);
			stream.close();
			return nullptr;
		});

		return nullptr;
	}, Stream::Type::Abortable);
}

