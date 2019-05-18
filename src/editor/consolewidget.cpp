#include "consolewidget.h"
#include "ui_consolewidget.h"
#include "editorcore.h"
#include "console.h"

static ConsoleWidget *instance;

void onEngineLog(const char *msg, LOG_TYPE type)
{
	QString color = "";

	if (type == LOG_TYPE::WARNING)
		color = " color=#bbbb00";
	if (type == LOG_TYPE::CRITICAL || type == LOG_TYPE::FATAL)
		color = " color=#cc0000";

	QString str = QString("<font")+ color +QString(">") + QString(msg) + QString("</font>");

	instance->ui->plainTextEdit->appendHtml(str);
}

ConsoleWidget::ConsoleWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConsoleWidget)
{
	instance = this;

	ui->setupUi(this);

	connect(editor, &EditorCore::OnEngineInstantiated, this, &ConsoleWidget::onEngineInstantiated, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineFree, this, &ConsoleWidget::onEngineFree, Qt::DirectConnection);
}

ConsoleWidget::~ConsoleWidget()
{
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


