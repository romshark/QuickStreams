#include "QuickStreamsTest.hpp"

// Verify the type-retry operator resurrects the stream for 2 times
// in case of matching (expected) error types before the stream finally closes
void QuickStreamsTest::retry_onType() {
	Trigger cpFailing;
	Trigger cpSecond;
	Trigger cpFailure;
	int counter(0);

	auto failingStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpFailing.trigger();
		++counter;
		// Commit failure here
		switch(counter) {
		case 1: throw std::logic_error("first error");
		case 2: throw std::runtime_error("second error");
		default: break;
		}
		stream.close();
	});

	failingStream->retry({
		exception::LogicError::type(),
		exception::RuntimeError::type(),
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
	QVERIFY(cpSecond.wait(100));
	QVERIFY(!cpFailure.wait(100));

	// Ensure first stream was executed for 3 times and the second just once.
	// The failure stream should not have been executed
	QCOMPARE(cpFailing.count(), 3);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpFailure.count(), 0);
}
