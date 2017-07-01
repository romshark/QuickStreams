#include "QuickStreamsTest.hpp"

void QuickStreamsTest::init() {
	// Even though this is an obvious memory leak (never deleting any provider)
	// its tolerable in tests.
	// The providers cannot be deleted by the test cleanup method because
	// some test functions don't wait until all streams finished and return. If
	// the providers needed by the streams created in those test functions are
	// deleted - read access violations can possibly crash the test.
	streams = new Provider;
}

QTEST_GUILESS_MAIN(QuickStreamsTest)
