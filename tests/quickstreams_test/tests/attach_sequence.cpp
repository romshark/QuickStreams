#include "QuickStreamsTest.hpp"

// Verify a sequence of 4 attached streams is executed properly
// and the data is passed correctly.
void QuickStreamsTest::attach_sequence() {
	Trigger cpFirst;
	Trigger cpSecond;
	Trigger cpThird;
	Trigger cpFourth;

	QList<QString> awakeningOrder;

	QVariant secondData(1);
	QVariant secondReceivedData;

	QVariant thirdData("test data");
	QVariant thirdReceivedData;

	QVariant fourthData(QVariantMap({{"[1]", 1}, {"[2]", 2}}));
	QVariant fourthReceivedData;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) -> Stream::Reference {
		Q_UNUSED(data)
		stream.close(secondData);
		cpFirst.trigger();
		awakeningOrder.append("first");
		return nullptr;
	});

	auto secondStream = firstStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		secondReceivedData = data;
		stream.close(thirdData);
		cpSecond.trigger();
		awakeningOrder.append("second");
		return nullptr;
	});

	auto thirdStream = secondStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		thirdReceivedData = data;
		stream.close(fourthData);
		cpThird.trigger();
		awakeningOrder.append("third");
		return nullptr;
	});

	auto fourthStream = thirdStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		fourthReceivedData = data;
		stream.close();
		cpFourth.trigger();
		awakeningOrder.append("fourth");
		return nullptr;
	});

	Q_UNUSED(fourthStream)

	// Ensure all streams were executed
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpSecond.wait(100));
	QVERIFY(cpThird.wait(100));
	QVERIFY(cpFourth.wait(100));

	// Ensure all streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpThird.count(), 1);
	QCOMPARE(cpFourth.count(), 1);

	// Ensure streams were executed in the right order
	QCOMPARE(awakeningOrder[0], QString("first"));
	QCOMPARE(awakeningOrder[1], QString("second"));
	QCOMPARE(awakeningOrder[2], QString("third"));
	QCOMPARE(awakeningOrder[3], QString("fourth"));

	// Ensure data was passed correctly
	QCOMPARE(secondData, secondReceivedData);
	QCOMPARE(thirdData, thirdReceivedData);
	QCOMPARE(fourthData, fourthReceivedData);
}
