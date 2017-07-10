#pragma once

#include "Stream.hpp"
#include "QmlStream.hpp"
#include "Provider.hpp"
#include "Error.hpp"
#include <QObject>
#include <QQmlEngine>
#include <QJSValue>
#include <QString>

namespace quickstreams {
namespace qml {

class ExceptionTypeList {
	Q_GADGET
	Q_PROPERTY(int Exception READ Exception CONSTANT)
	Q_PROPERTY(int JsException READ JsException CONSTANT)
	Q_PROPERTY(int BadTypeId READ BadTypeId CONSTANT)
	Q_PROPERTY(int BadCast READ BadCast CONSTANT)
	Q_PROPERTY(int BadWeakPtr READ BadWeakPtr CONSTANT)
	Q_PROPERTY(int BadFunctionCall READ BadFunctionCall CONSTANT)
	Q_PROPERTY(int BadAlloc READ BadAlloc CONSTANT)
	Q_PROPERTY(int BadArrayNewLength READ BadArrayNewLength CONSTANT)
	Q_PROPERTY(int BadException READ BadException CONSTANT)
	Q_PROPERTY(int LogicError READ LogicError CONSTANT)
	Q_PROPERTY(int InvalidArgument READ InvalidArgument CONSTANT)
	Q_PROPERTY(int DomainError READ DomainError CONSTANT)
	Q_PROPERTY(int LengthError READ LengthError CONSTANT)
	Q_PROPERTY(int OutOfRange READ OutOfRange CONSTANT)
	Q_PROPERTY(int FutureError READ FutureError CONSTANT)
	Q_PROPERTY(int RuntimeError READ RuntimeError CONSTANT)
	Q_PROPERTY(int RangeError READ RangeError CONSTANT)
	Q_PROPERTY(int OverflowError READ OverflowError CONSTANT)
	Q_PROPERTY(int UnderflowError READ UnderflowError CONSTANT)
	Q_PROPERTY(int RegexError READ RegexError CONSTANT)
	Q_PROPERTY(int SystemError READ SystemError CONSTANT)

public:
	static int Exception();
	static int JsException();
	static int BadTypeId();
	static int BadCast();
	static int BadWeakPtr();
	static int BadFunctionCall();
	static int BadAlloc();
	static int BadArrayNewLength();
	static int BadException();
	static int LogicError();
	static int InvalidArgument();
	static int DomainError();
	static int LengthError();
	static int OutOfRange();
	static int FutureError();
	static int RuntimeError();
	static int RangeError();
	static int OverflowError();
	static int UnderflowError();
	static int RegexError();
	static int SystemError();
};

class QmlProvider : public QObject {
	Q_OBJECT
	Q_PROPERTY(
		quickstreams::qml::ExceptionTypeList exceptions
		READ exceptions CONSTANT
	)
	Q_PROPERTY(quickstreams::Stream::Type Atomic READ Atomic CONSTANT)
	Q_PROPERTY(quickstreams::Stream::Type Abortable READ Abortable CONSTANT)
	Q_PROPERTY(
		quint64 totalCreated
		READ totalCreated NOTIFY totalCreatedChanged
	)
	Q_PROPERTY(
		quint64 totalExisting
		READ totalExisting NOTIFY totalExistingChanged
	)
	Q_PROPERTY(
		quint64 totalActive
		READ totalActive NOTIFY totalActiveChanged
	)

protected:
	Provider* _provider;
	QQmlEngine* _engine;

	static const quickstreams::qml::ExceptionTypeList exceptionTypes;
	quickstreams::qml::ExceptionTypeList exceptions() const;

public:
	explicit QmlProvider(QQmlEngine* engine, Provider* provider);

	QmlStream* toQml(const quickstreams::Stream::Reference& stream);

	Q_INVOKABLE QmlStream* create(
		const QJSValue& target,
		quickstreams::Stream::Type type = quickstreams::Stream::Type::Atomic
	);

	quickstreams::Stream::Type Atomic() const;
	quickstreams::Stream::Type Abortable() const;

	quint64 totalCreated() const;
	quint64 totalExisting() const;
	quint64 totalActive() const;

signals:
	void totalCreatedChanged();
	void totalExistingChanged();
	void totalActiveChanged();
};

}} // quickstreams::qml
