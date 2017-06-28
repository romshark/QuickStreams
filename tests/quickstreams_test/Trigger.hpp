#pragma once

#include <QObject>
#include <QSignalSpy>

class Trigger : public QObject {
	Q_OBJECT

protected:
	QSignalSpy _spy;

public:
	explicit Trigger();
	bool wait(int timeout);
	int count() const;

signals:
	void triggered();

public slots:
	void trigger();
};
