#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <QWidget>
#include "core.h"

namespace Ui {
class ConsoleWidget;
}

void onEngineLog(const char *msg, LOG_TYPE type);

class ConsoleWidget : public QWidget
{
	Q_OBJECT

	friend void onEngineLog(const char *msg, LOG_TYPE type);

public:
	explicit ConsoleWidget(QWidget *parent = nullptr);
	~ConsoleWidget();

private:
	Ui::ConsoleWidget *ui;

private slots:
	void onEngineInstantiated(Core *c);
	void onEngineFree(Core *c);
};

#endif // CONSOLEWIDGET_H
