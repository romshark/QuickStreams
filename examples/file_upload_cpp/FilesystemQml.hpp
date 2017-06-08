#pragma once

#include "Filesystem.hpp"
#include <QuickStreams>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

// This class represents a QML wrapper for the network filesystem class.
// It simply translates JavaScript values and returns QML stream wrappers.
class FilesystemQml : public QObject {
	Q_OBJECT

protected:
	// The actual network filesystem mock
	Filesystem* _fs;

	// A QML streams provider, used to translate C++ stream into QML streams.
	quickstreams::qml::QmlProvider* _qmlStreamProvider;

public:
	FilesystemQml(
		Filesystem* fs,
		quickstreams::qml::QmlProvider* qmlStreamProvider
	);

	// QML adaptation of Filesystem::allocateFile
	Q_INVOKABLE quickstreams::qml::QmlStream* allocateFile();

	// QML adaptation of Filesystem::write
	Q_INVOKABLE quickstreams::qml::QmlStream* write(const QVariantMap& args);

	// QML adaptation of Filesystem::remove
	Q_INVOKABLE quickstreams::qml::QmlStream* remove(const QVariantMap& args);

	// QML adaptation of Filesystem::uploadFile
	Q_INVOKABLE quickstreams::qml::QmlStream* uploadFile(
		const QVariantMap& args
	);
};
