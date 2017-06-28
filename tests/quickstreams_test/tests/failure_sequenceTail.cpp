#include "QuickStreamsTest.hpp"

// Verify the failure stream defined at the end of a sequence of 3 streams
// is awoken in case of a failure in the first stream
// and the sequence is broken (not continued)
void QuickStreamsTest::failure_sequenceTail() {
	Trigger cpFirst;
	Trigger cpSecond;
	Trigger cpThird;
	Trigger cpFailure;

	auto firstStream = streams->create([&](
		const StreamHandle& stream, const QVariant& data
	) -> Stream::Reference {
		Q_UNUSED(data)
		cpFirst.trigger();
		// Commit failure here
		throw std::runtime_error("something went wrong");
		stream.close();
		return nullptr;
	});

	auto secondStream = firstStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpSecond.trigger();
		stream.close();
		return nullptr;
	});

	auto thirdStream = secondStream->attach([&](
		const StreamHandle& stream, const QVariant& data
	) {
		Q_UNUSED(data)
		cpThird.trigger();
		stream.close();
		return nullptr;
	});

	auto failureStream = thirdStream->failure([&](
		const StreamHandle& stream, const QVariant& error
	) {
		Q_UNUSED(error)
		cpFailure.trigger();
		stream.close();
		return nullptr;
	});

	Q_UNUSED(failureStream)

	// Ensure first and failure streams are awoken
	QVERIFY(cpFirst.wait(100));
	QVERIFY(cpFailure.wait(100));

	// Ensure second and third streams are never awoken
	QVERIFY(!cpSecond.wait(100));
	QVERIFY(!cpThird.wait(100));

	// Ensure first and failure streams were only executed once
	QCOMPARE(cpFirst.count(), 1);
	QCOMPARE(cpFailure.count(), 1);

	// Ensure second and third streams were never executed
	QCOMPARE(cpSecond.count(), 0);
	QCOMPARE(cpThird.count(), 0);
}
