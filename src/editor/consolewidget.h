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
	static ConsoleWidget* instance;
public:
	explicit ConsoleWidget(QWidget *parent = nullptr);
	~ConsoleWidget();

	void ProcessMessageQueue();
	static ConsoleWidget* getInstance() { return instance; }

private:
	Ui::ConsoleWidget *ui;

private slots:
	void onEngineInstantiated(Core *c);
	void onEngineFree(Core *c);
	void on_clear_btn_clicked();
};

#endif // CONSOLEWIDGET_H
