#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QuickStreams>

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication app(argc, argv);

	auto engine(new QQmlApplicationEngine);

	// Create a new streams provider and expose it to qml
	// in the form of a global context property.
	// Pass the applications QML engine to it, it'll need it
	// to allow smooth interaction between C++ and QML.
	auto streamsProvider(new quickstreams::StreamProvider(engine, engine));
	engine->rootContext()->setContextProperty("QuickStreams", streamsProvider);

	engine->load(QUrl(QLatin1String("qrc:/main.qml")));

	return app.exec();
}
