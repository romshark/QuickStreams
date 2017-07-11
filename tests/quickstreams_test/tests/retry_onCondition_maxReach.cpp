#include "QuickStreamsTest.hpp"

// Verify the conditional retry operator resurrects the stream for max 2 time
// before the sequence fails due to max trial count reached
void QuickStreamsTest::retry_onCondition_maxReach() {
	Trigger cpFailing;
	Trigger cpSecond;
	Trigger cpFailure;
	Trigger cpRetry;

	auto failingStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		Q_UNUSED(stream)
		cpFailing.trigger();
		// Commit failure here
		throw std::runtime_error("unknown error");
	});

	failingStream->retry([&](const QVariant& error) {
		Q_UNUSED(error)
		cpRetry.trigger();
		return true;
	}, 2);

	failingStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpSecond.trigger();
		return QVariant();
	});

	failingStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		cpFailure.trigger();
		return QVariant();
	});

	// Verify initial trial
	QVERIFY(cpFailing.wait(100));

	// Verify first retrial
	QVERIFY(cpFailing.wait(100));

	// Verify second retrial
	QVERIFY(cpFailing.wait(100));
	QVERIFY(cpFailure.wait(100));
	QVERIFY(!cpSecond.wait(100));

	// Ensure the failing stream was executed 3 times and the second only once.
	// The retry condition callback should have been executed only 2 times,
	// whereas the failure stream should never have been executed
	QCOMPARE(cpRetry.count(), 2);
	QCOMPARE(cpFailing.count(), 3);
	QCOMPARE(cpFailure.count(), 1);
	QCOMPARE(cpSecond.count(), 0);
}
