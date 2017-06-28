#include <QtTest>
#include "Trigger.hpp"
#include <QuickStreams>

using namespace quickstreams;

class QuickStreamsTest : public QObject {
	Q_OBJECT

protected:
	Provider* streams;

private slots:
	void init();
	void cleanup();

	// Attach operator tests
	void attach();
	void attach_sequence();

	// Failure operator tests
	void failure_noFail();
	void failure_sequenceTail();
	void failure_recoverySequence();
	void failure_data_stdRuntimeError();
	void failure_data_string();

	// Memory tests
	void memory();
};
