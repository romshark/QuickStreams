#include "QuickStreamsTest.hpp"

// Verify the conditional retry operator resurrects the stream for 2 time
// before the stream finally closes
void QuickStreamsTest::retry_onCondition() {
	Trigger cpFailing;
	Trigger cpSecond;
	Trigger cpFailure;
	Trigger cpRetry;
	int counter(0);

	Error firstError;
	Error secondError;

	auto failingStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpFailing.trigger();
		++counter;
		// Commit failure for 2 times here
		switch(counter) {
		case 1: throw std::logic_error("first error");
		case 2: throw std::runtime_error("second error");
		default: break;
		}
		stream.close();
	});

	failingStream->retry([&](const QVariant& error) {
		cpRetry.trigger();
		switch(counter) {
		case 1: firstError = error.value<Error>(); break;
		case 2: secondError = error.value<Error>(); break;
		default: break;
		}
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
	QVERIFY(cpSecond.wait(100));
	QVERIFY(!cpFailure.wait(100));

	// Ensure the failing stream was executed 3 times and the second only once.
	// The retry condition callback should have been executed only 2 times,
	// whereas the failure stream should never have been executed
	QCOMPARE(cpRetry.count(), 2);
	QCOMPARE(cpFailing.count(), 3);
	QCOMPARE(cpSecond.count(), 1);
	QCOMPARE(cpFailure.count(), 0);

	// Ensure correct errors were received by the conditional retry callback
	QVERIFY(firstError.is(exception::LogicError::type()));
	QVERIFY(secondError.is(exception::RuntimeError::type()));
	QCOMPARE(firstError.message(), QString("first error"));
	QCOMPARE(secondError.message(), QString("second error"));
}
