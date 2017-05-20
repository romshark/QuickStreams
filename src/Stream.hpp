#pragma once

#include "StreamHandle.hpp"
#include <QObject>
#include <QJSValue>
#include <QQmlEngine>
#include <QVariant>
#include <QString>
#include <QMetaType>
#include <QSet>

namespace streams {

class Streams;

class Stream : public QObject {
	Q_OBJECT
	friend class Streams;

protected:
	QQmlEngine* _engine;
	bool _dead;
	QJSValue _function;
	// TODO: implement retry sample container
	QVariantList _retryErrSamples;
	qint32 _maxTrials;
	qint32 _currentTrial;
	QJSValue _repeatCondition;
	QSet<QString> _observedEvents;
	StreamHandle _handle;

	explicit Stream(
		QQmlEngine* engine,
		const QJSValue& function,
		QObject* parent = nullptr
	);

	void emitEvent(const QString& name, const QVariant& data);
	void emitClosed(const QVariant& data);
	void emitFailed(const QVariant& reason);
	bool verifyErrorListed(const QVariant& err) const;
	Stream* createSubsequentStream(const QJSValue& callback) const;

protected slots:
	// handles failure stream propagation from subordinate streams
	void handleFailureStreamPropagation(Stream* failureStream);

	// awakes this stream when a superordinate stream is closed
	void awake(QVariant data = QVariant());

	// awakes this stream when a superordinate stream is closed
	void awakeFromEvent(QString name, QVariant data = QVariant());

	// closes this stream when an awaited stream was closed
	void handleAwaitClosed(QVariant data = QVariant());

	// fails this stream when an awaited stream has failed
	void handleAwaitFailed(QVariant reason = QVariant());

signals:
	void eventEmitted(QString name, QVariant data);
	void closed(QVariant data);
	void failed(QVariant reason);
	void propagateFailureStream(Stream* failureStream);

public:

	//TODO: implement
	// retry is a stream operator, it repeats resurrecting the current stream
	// if either of the given error samples match the catched error.
	Q_INVOKABLE Stream* retry(
		const QVariant& samples,
		const QJSValue& maxTrials = QJSValue()
	);

	//TODO: implement
	// repeat is a stream operator, it repeats resurrecting the current stream
	// if the given condition returns true.
	Q_INVOKABLE Stream* repeat(const QJSValue& condition);

	// next is a stream operator, it creates a new stream that is awoken
	// when the current stream is successfuly closed.
	Q_INVOKABLE Stream* next(const QJSValue& target);

	//TODO: implement
	// event is a stream operator, it creates and returns a new stream
	// that is awoken when a certain set of events occures.
	Q_INVOKABLE Stream* event(const QVariant& name, QJSValue callback);

	// failure is a chain operator that acts upon the superordinate
	// stream chain. It returns a new stream that is awoken
	// when the superordinate stream chain fails.
	Q_INVOKABLE Stream* failure(const QJSValue& target);
};

} // streams

Q_DECLARE_METATYPE(streams::Stream*)
