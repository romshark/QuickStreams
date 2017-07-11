#include "QuickStreamsTest.hpp"

// Verify the conditional retry operator doesn't resurrect the stream
// in case the condition is false
void QuickStreamsTest::retry_onCondition_false() {
	Trigger cpFailing;
	Trigger cpFailure;
	Trigger cpSecond;

	auto failingStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		Q_UNUSED(stream)
		cpFailing.trigger();
		// Commit failure here
		throw std::runtime_error("runtime error");
	});

	failingStream->retry([&](const QVariant& error) {
		Q_UNUSED(error)
		return false;
	}, 1);

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

	// Ensure first and failure streams are awoken
	QVERIFY(cpFailing.wait(100));
	QVERIFY(cpFailure.wait(100));
	QVERIFY(!cpSecond.wait(100));

	// Ensure first and failure streams were only executed once
	QCOMPARE(cpFailing.count(), 1);
	QCOMPARE(cpFailure.count(), 1);
	QCOMPARE(cpSecond.count(), 0);
}
