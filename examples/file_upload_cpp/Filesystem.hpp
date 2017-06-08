#pragma once

#include <QObject>
#include <QuickStreams>
#include <QString>
#include <QVariant>

class Filesystem : public QObject {
	Q_OBJECT

public:
	// Error codes
	enum class Error {
		UnknownError,
		TimeoutError
	};

protected:
	// A stream provider to create streams with
	quickstreams::Provider* _streamProvider;

	// Min and max network latency
	int _minLatency;
	int _maxLatency;

	// Generates a random file id
	static QString randomId(int length);

	// Generates a random latency in the specified range.
	int randomLatency() const;

public:
	// Constructs a network filesystem mock
	// with a latency between 1 and 2 seconds
	Filesystem(
		quickstreams::Provider* streamProvider,
		int minLatency = 1000,
		int maxLatency = 2000
	);

	// Simulates an asynchronous file allocation operation over the network.
	// It's atomic, thus not abortable and doesn't emit any events.
	// The file is considered allocated when the returned stream is closed.
	quickstreams::Stream::Reference allocateFile();

	// Simulates an asynchronous file write operation over the network.
	// It's atomic, thus not abortable and doesn't emit any events.
	// The file chunk is considered written when the returned stream is closed.
	quickstreams::Stream::Reference write(
		const QString& fileId,
		int position,
		int length
	);

	// Simulates an asynchronous file removal operation over the network.
	// It's atomic, thus not abortable and doesn't emit any events.
	// File's considered removed when the returned stream is closed.
	quickstreams::Stream::Reference remove(const QString& fileId);

	// A complex asynchronous operation simulating an abortable file upload
	// operation over the network. Works fine even on unreliable networks.
	// Cleans up in case of an irrecoverable failure or abortion.
	quickstreams::Stream::Reference uploadFile(int fileSize, int chunkSize);
};
