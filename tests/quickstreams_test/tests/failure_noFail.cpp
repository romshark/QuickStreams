#include "QuickStreamsTest.hpp"

// Verify the failure stream is never executed
// when there's no failure catched in the sequence
void QuickStreamsTest::failure_noFail() {
	Trigger cpFirst;
	Trigger cpSecond;
	Trigger cpFailure;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		stream.close();
		cpFirst.trigger();
	});

	auto attachedStream = firstStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpSecond.trigger();
		return QVariant();
	});

	auto failureStream = attachedStream->failure([&](const QVariant& data) {
		Q_UNUSED(data)
		// This failure fallback stream should never be executed
		cpFailure.trigger();
		return QVariant();
	});

	Q_UNUSED(failureStream)

	// Ensure first and second streams were awoken
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpSecond.wait(100));

	// Ensure failure stream was never awoken
	QVERIFY(!cpFailure.wait(100));

	// Ensure first and second streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpFailure.count(), 0);
}
