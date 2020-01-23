#include "consolewidget.h"
#include "ui_consolewidget.h"
#include "editorcore.h"
#include "console.h"
#include <QQueue>

static QQueue<QPair<QString, LOG_TYPE>> messageQueue;
ConsoleWidget* ConsoleWidget::instance;

void onEngineLog(const char *msg, LOG_TYPE type)
{
	messageQueue.enqueue({msg, type});
}

void ConsoleWidget::ProcessMessageQueue()
{
	while (!messageQueue.isEmpty())
	{
		auto& message = messageQueue.head();
		QString color = "";

		if (message.second == LOG_TYPE::WARNING)
			color = " color=#bbbb00";
		else if (message.second == LOG_TYPE::CRITICAL || message.second == LOG_TYPE::FATAL)
			color = " color=#cc0000";

		QString str = QString("<font")+ color +QString(">") + message.first + QString("</font>");

		instance->ui->plainTextEdit->appendHtml(str);
		messageQueue.dequeue();
	}
}

ConsoleWidget::ConsoleWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConsoleWidget)
{
	assert(instance == nullptr);
	instance = this;

	ui->setupUi(this);

	connect(editor, &EditorCore::OnEngineInstantiated, this, &ConsoleWidget::onEngineInstantiated, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineFree, this, &ConsoleWidget::onEngineFree, Qt::DirectConnection);
}

ConsoleWidget::~ConsoleWidget()
{
	instance = 0;
	delete ui;
}

void ConsoleWidget::onEngineInstantiated(Core *c)
{
	Console *console = c->GetConsole();
	console->AddCallback(onEngineLog);
}

void ConsoleWidget::onEngineFree(Core *c)
{
	//Console *console = c->GetConsole();
	//console->RemoveCallback(onEngineLog);
}



void ConsoleWidget::on_clear_btn_clicked()
{
	ui->plainTextEdit->clear();
}
