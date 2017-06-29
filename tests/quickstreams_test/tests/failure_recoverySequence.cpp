#include "QuickStreamsTest.hpp"

// Verify the failure stream properly awakes attached streams
void QuickStreamsTest::failure_recoverySequence() {
	Trigger cpFirst;
	Trigger cpFailure;
	Trigger cpFirstRecovery;
	Trigger cpSecondRecovery;

	QList<QString> awakeningOrder;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) -> Stream::Reference {
		Q_UNUSED(data)
		awakeningOrder.append("first");
		cpFirst.trigger();
		// Commit failure here
		throw std::runtime_error("something went wrong");
		stream.close();
		return nullptr;
	});

	auto failureStream = firstStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		awakeningOrder.append("failure");
		cpFailure.trigger();
		return QVariant();
	});

	auto firstRecoveryStream = failureStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		awakeningOrder.append("recovery - first attached");
		cpFirstRecovery.trigger();
		return QVariant();
	});

	auto secondRecoveryStream = firstRecoveryStream->bind([&](
		const QVariant& data
	) {
		Q_UNUSED(data)
		awakeningOrder.append("recovery - second attached");
		cpSecondRecovery.trigger();
		return QVariant();
	});

	Q_UNUSED(secondRecoveryStream)

	// Ensure first and failure streams are awoken
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpFailure.wait(100));
	QVERIFY(cpFirstRecovery.wait(100));
	QVERIFY(cpSecondRecovery.wait(100));

	// Ensure first and failure streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpFailure.count(), 1);
	QCOMPARE(cpFirstRecovery.count(), 1);
	QCOMPARE(cpSecondRecovery.count(), 1);

	// Ensure streams were awoken in the right order
	QCOMPARE(awakeningOrder[0], QString("first"));
	QCOMPARE(awakeningOrder[1], QString("failure"));
	QCOMPARE(awakeningOrder[2], QString("recovery - first attached"));
	QCOMPARE(awakeningOrder[3], QString("recovery - second attached"));
}
