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
	) -> Stream::Reference {
		Q_UNUSED(data)
		stream.close();
		cpFirst.trigger();
		return nullptr;
	});

	auto secondStream = firstStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		stream.close();
		cpSecond.trigger();
		return nullptr;
	});

	auto thirdStream = secondStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		stream.close();
		cpThird.trigger();
		return nullptr;
	});

	auto fourthStream = thirdStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		stream.close();
		cpFourth.trigger();
		return nullptr;
	});

	auto failureStream = fourthStream->failure([&](
		const StreamHandle& stream, const QVariant& error
	) {
		Q_UNUSED(error)
		stream.close();
		cpFailure.trigger();
		return nullptr;
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
