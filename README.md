# QuickStreams
An asynchronous programming library for the [QML](https://en.wikipedia.org/wiki/QML) programming language
inspired by [Reactive Extensions](http://reactivex.io/),
implementing the [Reactive Programming Paradigm](https://en.wikipedia.org/wiki/Reactive_programming)
in a slightly redesigned manner.

*First beta release, not yet production ready*

## Asynchronous Programming in Streams

[QuickStreams](https://qbeon.github.io/QuickStreams/) makes asynchronous programming in QML easier and safer by providing a way
to build consistent abstractions of complex asynchronous operations.
In contrast to Qt Quick's low level [Signals & Slots](http://doc.qt.io/qt-5/signalsandslots.html) mechanism,
primitive [Callbacks](https://en.wikipedia.org/wiki/Callback_(computer_programming))
and the more sophisticated [Promises and Futures](https://en.wikipedia.org/wiki/Futures_and_promises) -
**Streams** are suitable for a much wider spectrum of problems and also offer a much wider set of useful features.

For a more detailed description, please visit [qbeon.github.io/QuickStreams](https://qbeon.github.io/QuickStreams/)

## Installation

1. Clone this repository or download a [release version](https://github.com/qbeon/QuickStreams/releases)
to a folder within your projects source tree.

2. Add the **QuickStreams.pri** file to your project profile *.pro* file:
```
include(QuickStreams/QuickStreams.pri) # optionally adjust the path
```

3. Create a *StreamsProvider* object and expose it to QML as a context property:
```
// in your projects main.cpp

#include <QuickStreams>

int main() {
	//...
	
	auto engine(new QQmlApplicationEngine());
	
	// pass an engine pointer to the stream provider
	auto streamProvider(new quickstreams::StreamProvider(engine, engine));
	
	engine.rootContext()->setContextProperty("QuickStreams", streamProvider);
	
	//...
}
```

## Examples

You can find complete code examples in the
[examples sub-directory](https://github.com/qbeon/QuickStreams/tree/master/examples/file_upload)

For a detailed description of the streams programming paradigm, the use cases and the features, please visit
[qbeon.github.io/QuickStreams](https://qbeon.github.io/QuickStreams/)

## Feedback, Help & Contribution

* Ask questions on Stack Overflow
* Request a new feature on GitHub
* Report a bug in GitHub Issues.
* Join our [Discussion Group on Telegram](https://t.me/qt_chat)
* Or contact us directly via contact@qbeon.com

## License

Copyright (c) Roman Sharkov. All rights reserved.

Licensed under the MIT License.
