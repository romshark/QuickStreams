#include "QuickStreamsTest.hpp"

// Verify all sequences (regular control flow, failure and abortion branches)
// are properly initialized when the initial free stream is scheduled for
// execution and control flow manipulation at runtime is forbidden.
void QuickStreamsTest::sequenceInitialization() {
	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		QTimer::singleShot(10, [stream]{
			stream.close();
		});
	});

	auto secondStream = firstStream->attach([&](const QVariant& data) {
		Q_UNUSED(data)
		return QVariant();
	});

	auto failureStream = secondStream->failure([&](const QVariant& error) {
		Q_UNUSED(error)
		return QVariant();
	});

	auto abortionStream = secondStream->abortion([&](const QVariant& data) {
		Q_UNUSED(data)
		return QVariant();
	});

	QCOMPARE(firstStream->state(), Stream::State::Initializing);
	QCOMPARE(secondStream->state(), Stream::State::Initializing);
	QCOMPARE(failureStream->state(), Stream::State::Initializing);
	QCOMPARE(abortionStream->state(), Stream::State::Initializing);

	// Await next event loop cycle
	Trigger awaitEventLoopCycle;
	QTimer::singleShot(1, [&] {
		awaitEventLoopCycle.trigger();
	});
	QVERIFY(awaitEventLoopCycle.wait(1));

	QCOMPARE(firstStream->state(), Stream::State::Active);
	QCOMPARE(secondStream->state(), Stream::State::Awaiting);
	QCOMPARE(failureStream->state(), Stream::State::Awaiting);
	QCOMPARE(abortionStream->state(), Stream::State::Awaiting);

	// Verify attach-, bind-, failure- and abortion-operator exceptions
	auto sampleFunction([](const QVariant& d) {
		Q_UNUSED(d)
		return QVariant();
	});

	QVERIFY_EXCEPTION_THROWN(
		firstStream->attach(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		firstStream->bind(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		firstStream->failure(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		firstStream->abortion(sampleFunction),
		std::logic_error
	);

	QVERIFY_EXCEPTION_THROWN(
		secondStream->attach(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		secondStream->bind(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		secondStream->failure(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		secondStream->abortion(sampleFunction),
		std::logic_error
	);

	QVERIFY_EXCEPTION_THROWN(
		failureStream->attach(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		failureStream->bind(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		failureStream->failure(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		failureStream->abortion(sampleFunction),
		std::logic_error
	);

	QVERIFY_EXCEPTION_THROWN(
		abortionStream->attach(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		abortionStream->bind(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		abortionStream->failure(sampleFunction),
		std::logic_error
	);
	QVERIFY_EXCEPTION_THROWN(
		abortionStream->abortion(sampleFunction),
		std::logic_error
	);
}
