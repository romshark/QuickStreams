# QuickStreams
An asynchronous programming library for the [QML](https://en.wikipedia.org/wiki/QML) and C++ programming languages
inspired by [Reactive Extensions](http://reactivex.io/),
implementing the asynchronous streams paradigm - an advanced, yet simple, communication protocol for various application components.

*First beta release, not yet production ready*

## Asynchronous Programming in Streams

[QuickStreams](https://qbeon.github.io/QuickStreams/) makes asynchronous programming in C++ & QML easier and safer by providing a way
to build consistent abstractions of complex asynchronous operations and transactions.


In contrast to Qt Quick's low level [Signals & Slots](http://doc.qt.io/qt-5/signalsandslots.html) mechanism,
primitive [Callbacks](https://en.wikipedia.org/wiki/Callback_(computer_programming))
and the more sophisticated [Promises and Futures](https://en.wikipedia.org/wiki/Futures_and_promises) -
**Streams** are suitable for a much wider spectrum of problems and also offer a much wider set of useful features.


**Streams** can...
- abstract away asynchronous (abortable and atomic) operations and transactions
- be chained into asynchronous sequences
- abstract away sequences of other streams
- safely recover from unexpected errors
- be retried on expected errors
- be aborted
- be repeated conditionally (asynchronous loops)
- emit events

For a more detailed description, please visit [qbeon.github.io/QuickStreams](https://qbeon.github.io/QuickStreams/)

## Installation

1. Clone this repository or download a [release version](https://github.com/qbeon/QuickStreams/releases)
to a folder within your projects source tree.

2. Add the **QuickStreams.pri** file to your project profile *.pro* file:
```
include(QuickStreams/QuickStreams.pri) # optionally adjust the path
```

3. Create a *quickstreams::Provider* object and inject it (by reference) into any C++ component to create streams in. Optionally create a *quickstreams::qml::QmlProvider* and expose it to QML as a context property to create streams in QML:
```
// in your projects main.cpp

#include <QuickStreams>

int main() {
	//...
	
	auto engine(new QQmlApplicationEngine);

	// Create a streams provider
	auto provider(new quickstreams::Provider);

	// Create a new QML streams provider wrapping a regular streams provider.
	auto qmlProvider(new quickstreams::qml::QmlProvider(engine, provider));
	
	// Expose the QML streams provider to QML
	// to enable streams creation from from QML
	engine.rootContext()->setContextProperty("QuickStreams", qmlProvider);
	
	//...
}
```
*quickstreams::qml::QmlProvider* should be injected (by reference) into C++ components exposing APIs to QML.

## Examples

- [Network Filesystem API](https://github.com/qbeon/QuickStreams/tree/master/examples/file_upload) - A network filesystem simulation implemented in pure QML JavaScript. Represents the unreliable, asynchronous filesystem API in streams.

- [C++ API exposed to QML](https://github.com/qbeon/QuickStreams/tree/master/examples/file_upload_cpp) - Similar to the [network filesystem API](https://github.com/qbeon/QuickStreams/tree/master/examples/file_upload) example, but implements the network filesystem API in C++ exposing it to QML using the *quickstreams::qml::QmlProvider*.

For a detailed description of the streams programming paradigm, use cases and features, please visit
[qbeon.github.io/QuickStreams](https://qbeon.github.io/QuickStreams/)

## Feedback, Help & Contribution

* Ask questions and follow discussions on [Stack Overflow](https://stackoverflow.com/), [QtMob](https://qtmob.slack.com), [Telegram (RU)](https://t.me/qt_chat)
* Request new features and report bugs in [GitHub Issues](https://github.com/qbeon/QuickStreams/issues).
* Or contact us directly via contact@qbeon.com

## License

Copyright (c) Roman Sharkov. All rights reserved.

Licensed under the MIT License.
