#include "QuickStreamsTest.hpp"

// Verify the failure sequence defined at the beginning of a sequence
// is awoken in case of a failure and the sequence is broken (not continued)
void QuickStreamsTest::failure_sequenceHead() {
	Trigger cpFirst;
	Trigger cpSecond;
	Trigger cpThird;
	Trigger cpFourth;
	Trigger cpFailure;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpFirst.trigger();
		stream.close();
	});

	// The second, third and fourth streams must inherit this failure stream
	auto failureStream = firstStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		cpFailure.trigger();
		return QVariant();
	});

	auto secondStream = firstStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpSecond.trigger();
		return QVariant();
	});

	auto thirdStream = secondStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpThird.trigger();
		// Commit failure here
		throw std::runtime_error("something went wrong");
		return QVariant();
	});

	auto fourthStream = thirdStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpFourth.trigger();
		return QVariant();
	});

	Q_UNUSED(failureStream)
	Q_UNUSED(fourthStream)

	// Ensure first, second, third and failure streams are awoken
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpSecond.wait(100));
	QVERIFY(cpThird.wait(100));
	QVERIFY(cpFailure.wait(100));

	// Ensure fourth stream is never awoken
	QVERIFY(!cpFourth.wait(100));

	// Ensure first and failure streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpThird.count(), 1);
	QCOMPARE(cpFailure.count(), 1);

	// Ensure second and third streams were never executed
	QCOMPARE(cpFourth.count(), 0);
}
