#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QuickStreams>

#include "Filesystem.hpp"
#include "FilesystemQml.hpp"

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication app(argc, argv);

	auto engine(new QQmlApplicationEngine);

	// Create a streams provider
	auto provider(new quickstreams::Provider);

	// Create a new QML streams provider based on the regular stream provider
	// and expose it to QML in the form of a global context property.
	// This will allow to create streams in QML
	auto qmlProvider(new quickstreams::qml::QmlProvider(engine, provider));

	// Create a network filesystem mock instance
	auto fs(new Filesystem(provider));

	// Create a QML wrapper for the network filesystem mock
	auto fsQml(new FilesystemQml(fs, qmlProvider));

	auto context(engine->rootContext());
	context->setContextProperty("QuickStreams", qmlProvider);
	context->setContextProperty("Filesystem", fsQml);

	engine->load(QUrl(QLatin1String("qrc:/main.qml")));

	return app.exec();
}
