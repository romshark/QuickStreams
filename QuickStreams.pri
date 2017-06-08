QT += qml
CONFIG += c++11

INCLUDEPATH += $$PWD/src

HEADERS += \
	$$PWD/src/Stream.hpp \
	$$PWD/src/StreamHandle.hpp \
	$$PWD/src/Provider.hpp \
	$$PWD/src/QmlProvider.hpp \
	$$PWD/src/LambdaExecutable.hpp \
	$$PWD/src/JsExecutable.hpp \
	$$PWD/src/QmlStream.hpp \
	$$PWD/src/Retryer.hpp \
	$$PWD/src/LambdaRepeater.hpp \
	$$PWD/src/JsRepeater.hpp \
	$$PWD/src/Callback.hpp \
	$$PWD/src/JsCallback.hpp \
	$$PWD/src/LambdaCallback.hpp \
	$$PWD/src/Executable.hpp \
	$$PWD/src/QmlStreamHandle.hpp \
	$$PWD/src/Repeater.hpp \
	$$PWD/src/ProviderInterface.hpp

SOURCES += \
	$$PWD/src/Stream.cpp \
	$$PWD/src/StreamHandle.cpp \
	$$PWD/src/Provider.cpp \
	$$PWD/src/QmlProvider.cpp \
	$$PWD/src/LambdaExecutable.cpp \
	$$PWD/src/JsExecutable.cpp \
	$$PWD/src/QmlStream.cpp \
	$$PWD/src/Retryer.cpp \
	$$PWD/src/LambdaRepeater.cpp \
	$$PWD/src/JsRepeater.cpp \
	$$PWD/src/JsCallback.cpp \
	$$PWD/src/LambdaCallback.cpp \
	$$PWD/src/Executable.cpp \
	$$PWD/src/QmlStreamHandle.cpp

DISTFILES += \
    $$PWD/README.md
