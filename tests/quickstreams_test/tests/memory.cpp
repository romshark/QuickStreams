#include "QuickStreamsTest.hpp"

// Verify a sequence of 4 attached streams is executed properly
// and the data is passed correctly.
void QuickStreamsTest::memory() {
	Trigger cpFirst;
	Trigger cpSecond;
	Trigger cpThird;
	Trigger cpFourth;
	Trigger cpFailure;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		stream.close();
		cpFirst.trigger();
	});

	auto secondStream = firstStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpSecond.trigger();
		return QVariant();
	});

	auto thirdStream = secondStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpThird.trigger();
		return QVariant();
	});

	auto fourthStream = thirdStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		cpFourth.trigger();
		return QVariant();
	});

	auto failureStream = fourthStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		cpFailure.trigger();
		return QVariant();
	});

	QCOMPARE(streams->totalCreated(), quint64(5));
	QCOMPARE(streams->totalExisting(), quint64(5));
	QCOMPARE(streams->totalActive(), quint64(0));

	// Ensure all streams except failure were executed
	QVERIFY(cpFirst.wait(50));
	QVERIFY(cpSecond.wait(50));
	QVERIFY(cpThird.wait(50));
	QVERIFY(cpFourth.wait(50));
	QVERIFY(!cpFailure.wait(50));

	// Ensure all streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpThird.count(), 1);
	QCOMPARE(cpFourth.count(), 1);
	QCOMPARE(cpFailure.count(), 0);

	// Even though all referenced perished deleteLater will still
	// delete the streams in a later event loop cycle
	QCOMPARE(streams->totalCreated(), quint64(5));
	QCOMPARE(streams->totalExisting(), quint64(5));
	QCOMPARE(streams->totalActive(), quint64(0));

	firstStream.clear();
	secondStream.clear();
	thirdStream.clear();
	fourthStream.clear();

	// Await next event loop cycle
	Trigger cleanup;
	QTimer::singleShot(1, [&] {
		cleanup.trigger();
	});
	QVERIFY(cleanup.wait(1));

	// Now all that deleteLater was executed the streams should be gone
	QCOMPARE(streams->totalCreated(), quint64(5));
	QCOMPARE(streams->totalExisting(), quint64(0));
	QCOMPARE(streams->totalActive(), quint64(0));
}
