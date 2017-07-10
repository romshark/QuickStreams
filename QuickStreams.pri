QT += qml
CONFIG += c++11

INCLUDEPATH += $$PWD/src

HEADERS += \
	$$PWD/src/Stream.hpp \
	$$PWD/src/StreamHandle.hpp \
	$$PWD/src/Provider.hpp \
	$$PWD/src/QmlProvider.hpp \
	$$PWD/src/LambdaExecutable.hpp \
	$$PWD/src/LambdaSyncExecutable.hpp \
	$$PWD/src/LambdaWrapper.hpp \
	$$PWD/src/JsExecutable.hpp \
	$$PWD/src/JsSyncExecutable.hpp \
	$$PWD/src/QmlStream.hpp \
	$$PWD/src/LambdaRepeater.hpp \
	$$PWD/src/JsRepeater.hpp \
	$$PWD/src/Callback.hpp \
	$$PWD/src/JsCallback.hpp \
	$$PWD/src/LambdaCallback.hpp \
	$$PWD/src/Executable.hpp \
	$$PWD/src/QmlStreamHandle.hpp \
	$$PWD/src/Repeater.hpp \
	$$PWD/src/ProviderInterface.hpp \
	$$PWD/src/Retryer.hpp \
	$$PWD/src/LambdaRetryer.hpp \
	$$PWD/src/TypeRetryer.hpp \
	$$PWD/src/Error.hpp \
	$$PWD/src/JsTypeRetryer.hpp \
	$$PWD/src/JsConditionRetryer.hpp

SOURCES += \
	$$PWD/src/Stream.cpp \
	$$PWD/src/StreamHandle.cpp \
	$$PWD/src/Provider.cpp \
	$$PWD/src/QmlProvider.cpp \
	$$PWD/src/LambdaExecutable.cpp \
	$$PWD/src/LambdaSyncExecutable.cpp \
	$$PWD/src/LambdaWrapper.cpp \
	$$PWD/src/JsExecutable.cpp \
	$$PWD/src/JsSyncExecutable.cpp \
	$$PWD/src/QmlStream.cpp \
	$$PWD/src/LambdaRepeater.cpp \
	$$PWD/src/JsRepeater.cpp \
	$$PWD/src/JsCallback.cpp \
	$$PWD/src/LambdaCallback.cpp \
	$$PWD/src/Executable.cpp \
	$$PWD/src/QmlStreamHandle.cpp \
	$$PWD/src/Retryer.cpp \
	$$PWD/src/LambdaRetryer.cpp \
	$$PWD/src/TypeRetryer.cpp \
	$$PWD/src/Error.cpp \
	$$PWD/src/JsTypeRetryer.cpp \
	$$PWD/src/JsConditionRetryer.cpp

DISTFILES += \
    $$PWD/README.md
