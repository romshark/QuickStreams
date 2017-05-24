QT += qml
CONFIG += c++11

INCLUDEPATH += $$PWD/src

HEADERS += \
	$$PWD/src/Stream.hpp \
	$$PWD/src/StreamHandle.hpp \
	$$PWD/src/StreamProvider.hpp

SOURCES += \
	$$PWD/src/Stream.cpp \
	$$PWD/src/StreamHandle.cpp \
	$$PWD/src/StreamProvider.cpp

DISTFILES += \
    $$PWD/README.md
