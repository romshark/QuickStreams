#include "QuickStreamsTest.hpp"

void QuickStreamsTest::init() {
	streams = new Provider;
}

void QuickStreamsTest::cleanup() {
	streams->deleteLater();
}

QTEST_GUILESS_MAIN(QuickStreamsTest)
