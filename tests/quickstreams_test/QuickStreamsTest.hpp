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

	// Attach operator tests
	void attach();
	void attach_sequence();

	// Failure operator tests
	void failure_noFail();
	void failure_sequenceTail();
	void failure_sequenceHead();
	void failure_override();
	void failure_recoverySequence();
	void failure_data_stdRuntimeError();
	void failure_data_string();

	// Retry operator tests
	void retry_onCondition();
	void retry_onCondition_false();
	void retry_onCondition_maxReach();
	void retry_onType();
	void retry_onType_mismatchTypes();
	void retry_onType_maxReach();

	// Memory and state management tests
	void sequenceInitialization();
	void memory();
};
