#include "FilesystemQml.hpp"
#include <QuickStreams>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

FilesystemQml::FilesystemQml(
	Filesystem* fs,
	quickstreams::qml::QmlProvider* qmlStreamProvider
) :
	QObject(nullptr),
	_fs(fs),
	_qmlStreamProvider(qmlStreamProvider)
{

}

quickstreams::qml::QmlStream* FilesystemQml::allocateFile() {
	return _qmlStreamProvider->toQml(_fs->allocateFile());
}

quickstreams::qml::QmlStream* FilesystemQml::write(const QVariantMap& args) {
	return _qmlStreamProvider->toQml(_fs->write(
		args["fileId"].toString(),
		args["position"].toInt(),
		args["length"].toInt()
	));
}

quickstreams::qml::QmlStream* FilesystemQml::remove(const QVariantMap& args) {
	return _qmlStreamProvider->toQml(_fs->remove(
		args["fileId"].toString()
	));
}

quickstreams::qml::QmlStream* FilesystemQml::uploadFile(
	const QVariantMap& args
) {
	return _qmlStreamProvider->toQml(_fs->uploadFile(
		args["fileSize"].toInt(),
		args["chunkSize"].toInt()
	));
}
