QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    QuickStreamsTest.cpp \
    Trigger.cpp \
    tests/attach.cpp \
    tests/failure_sequenceTail.cpp \
    tests/failure_data_stdRuntimeError.cpp \
    tests/failure_noFail.cpp \
    tests/failure_data_string.cpp \
    tests/failure_recoverySequence.cpp \
    tests/attach_sequence.cpp \
    tests/memory.cpp \
    tests/sequenceInitialization.cpp \
    tests/failure_override.cpp \
    tests/failure_sequenceHead.cpp \
    tests/retry_onType_mismatchTypes.cpp \
    tests/retry_onType.cpp \
    tests/retry_onCondition.cpp \
    tests/retry_onCondition_false.cpp \
    tests/retry_onCondition_maxReach.cpp \
    tests/retry_onType_maxReached.cpp

HEADERS += \
    Trigger.hpp \
    QuickStreamsTest.hpp

include(../../QuickStreams.pri)
