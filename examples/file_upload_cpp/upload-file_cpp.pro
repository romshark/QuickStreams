QT += qml quick

CONFIG += c++11

SOURCES += main.cpp \
    Filesystem.cpp \
    FilesystemQml.cpp \
    fs_write.cpp \
    fs_remove.cpp \
    fs_allocateFile.cpp \
    fs_uploadFile.cpp
RESOURCES += qml.qrc

QML_IMPORT_PATH = "qrc:/"

DEFINES += QT_DEPRECATED_WARNINGS

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# include QuickStreams as a dependency
include(../../QuickStreams.pri)

HEADERS += \
    Filesystem.hpp \
    FilesystemQml.hpp
