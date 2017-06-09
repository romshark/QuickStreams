import QtQuick 2.7

Item {
	id: filesystem

	Helper {
		id: helper
	}

	// Environment variables
	property int minNetworkLatency: 500
	property int maxNetworkLatency: 1000

	// Error samples
	property var timeoutError: new Error('timeout')

	// File allocation method. Returns an atomic stream not emitting any events
	// providing the id of the created file when closed
	property var allocateFile: function() {
		// Create the main atomic stream to be returned
		return QuickStreams.create(function(mainStream) {
			console.log('FS::allocateFile > allocate file')

			// Create the allocOperation atomic stream
			var allocOperation = QuickStreams.create(function() {
				var newId = helper.randomId(8)
				console.log('FS::allocateFile > file allocated', newId)

				// Close the returned main stream
				mainStream.close(newId)
			})

			// Delay the awakening of the allocOperation stream
			allocOperation.delay(helper.randomNetLatency())

			// Make the allocOperation stream subordinate
			// to the main stream that's returned by this function
			mainStream.adopt(allocOperation)
		})
	}

	// File writing method, returns an atomic stream not emitting any events.
	// The operation is considered completed when the returned stream is closed
	property var write: function(arg) {
		// Create the main atomic stream to be returned
		return QuickStreams.create(function(mainStream) {
			console.log(
				'FS::write > write to file:', arg.file,
				'at byte', arg.position,
				'for', arg.length, 'bytes'
			)

			// Create the writeOperation atomic stream,
			// Don't worry about this stream not being closed,
			// It won't become a zombie, because the mainStream
			// is properly closed and will erase the adopted stream on closure
			return QuickStreams.create(function() {
				console.log('FS::write > write completed')
				mainStream.close()
			})
			.delay(helper.randomNetLatency())
		})
	}

	// File removal method, returns an atomic stream not emitting any events.
	// The operation is considered completed when the returned stream is closed
	property var remove: function(fileId) {
		return QuickStreams.create(function(mainStream) {
			console.log('FS::remove > remove file', fileId, '...')

			// Return a stream during the execution of mainStream
			// will make mainStream wrap this stream
			// making them both become one
			return QuickStreams.create(function() {
				console.log('FS::remove > file', fileId, 'has been removed')
				mainStream.close()
			})
			.delay(helper.randomNetLatency())
		})
	}

	// Abortable, continuous file upload method.
	// The file is considered completely uploaded
	// when the returned stream is closed.
	// Emits 'allocated' on successful file allocation.
	// Emits 'chunk_uploaded' events on chunk upload completion
	property var uploadFile: function(args) {
		// Set milestones for eventual cleanup
		var allocated = false

		// Define context variables for progress tracking
		var uploadProgress = 0
		var fileId

		console.log('FS::uploadFile > begin new upload')

		// Create a new, abortable, event emitting stream
		// to abstract away a complex, underlying stream chain
		return QuickStreams.create(function(mainStream) {

			// mainStream is the parent of the following stream chain.
			// This setting is important for proper abortion handling.
			// When mainStream is requested to abort - this underlying chain
			// should properly abort execution and clean up if necessary

			// Prepare upload by allocating the file on the filesystem.
			// This operation will return an atomic stream on top of which
			// the following chain is build
			var chain = mainStream.adopt(filesystem.allocateFile())

			// Indefinitely retry file allocation in case of timeout errors
			.retry([filesystem.TimeoutError])

			// Bind this asynchronous operation, it should be executed
			// no matter whether the chain was aborted in the meantime or not.
			// It will ensure that the file allocation milestone is marked
			// as achieved even if the chain was aborted while the filesystem
			// was allocating the file for proper clean up
			.bind(function(stream, newFileId) {
				// mark allocation milestone
				allocated = true

				// Remember fileId for later and emit 'allocated' event
				// after a successful file allocation
				fileId = newFileId
				mainStream.event('allocated', {id: fileId})
				stream.close()
			})

			// Begin uploading the file chunk after chunk
			// when the file has successfuly been allocated
			.attach(function() {
				return filesystem.write({
					file: fileId,
					position: uploadProgress,
					length: args.chunkSize
				})
			})

			// Indefinitely retry writing chunks in case of timeout errors
			.retry([filesystem.TimeoutError])

			// Repeat writing until all chunks are written.
			// Write operations are atomic streams and will repeat even
			// if the file upload was aborted. To prevent the atomic stream
			// from repeating after abortion the "aborted" argument must be
			// checked manually to stop repetition in case of upload abortion
			.repeat(function(aborted) {
				// update upload progress and emit 'chunk_uploaded' event
				// after each successful write
				uploadProgress += args.chunkSize
				mainStream.event('chunk_uploaded', {
					progress: uploadProgress
				})

				// If upload was aborted then stop repeating write
				if(aborted) return false

				// Continue repeating if the file isn't yet fully uploaded
				if(uploadProgress < args.fileSize) return true

				// Otherwise stop writing
				return false
			})

			// Close main stream when all chunks have successfuly been written
			.attach(function(stream) {
				mainStream.close(fileId)
			})

			// Clean up in case the started file upload must be undone
			function cleanup(error) {
				// There's nothing to clean up if file wasn't yet allocated.
				// Otherwise the allocated file must be removed
				if(!allocated) {
					if(error) {
						mainStream.fail(error)
						return
					}
					mainStream.close()
				}

				// Try to remove the unfinished file
				filesystem.remove(fileId)
				.attach(function(stream) {
					mainStream.close()
				})
				// Catch unexpected clean up errors
				// and fail the main stream in such a case
				.failure(function(stream, error) {
					mainStream.fail(error)
				})

				// This console output visualizes the end
				// of the declaration tick
				console.log('--------- CLEANUP CHAIN DECLARED ---------')
			}

			// Catch unexpected errors during chain execution
			// and clean up before failing the main stream
			chain.failure(function(stream, error) {
				console.log('FS::uploadFile > catch failed file upload')
				mainStream.event('cleanup_after_error', error)
				cleanup(error)
			})

			// Clean up before abort-closing the main stream
			// if the upload chain was aborted
			chain.abortion(function(stream) {
				console.log('FS::uploadFile > catch aborted file upload')
				mainStream.event('cleanup_after_abortion')
				cleanup()
			})

			// This console output visualizes the end of the declaration tick
			console.log('--------- FILEUPLOAD CHAIN DECLARED  ---------')

		}, QuickStreams.Abortable)
	}
}
