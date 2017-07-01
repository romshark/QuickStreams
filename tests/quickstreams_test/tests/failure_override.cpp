#include "QuickStreamsTest.hpp"

// Verify the failure stream defined at the end of a sequence of 3 streams
// is awoken in case of a failure in the first stream
// and the sequence is broken (not continued)
void QuickStreamsTest::failure_override() {
	Trigger cpFirst;
	Trigger cpFailure_overridden;
	Trigger cpFailure_actual;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpFirst.trigger();
		// Commit failure here
		throw std::runtime_error("something went wrong");
		stream.close();
	});

	auto failureStream1 = firstStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		cpFailure_overridden.trigger();
		return QVariant();
	});

	auto failureStream2 = firstStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		cpFailure_actual.trigger();
		return QVariant();
	});

	Q_UNUSED(failureStream1)
	Q_UNUSED(failureStream2)

	// Ensure first and actual failure streams are awoken
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpFailure_actual.wait(100));

	// Ensure overridden failure stream is never awoken
	QVERIFY(!cpFailure_overridden.wait(100));

	// Ensure first and actual failure streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpFailure_actual.count(), 1);

	// Ensure overridden failure stream was never executed
	QCOMPARE(cpFailure_overridden.count(), 0);
}
