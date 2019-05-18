#include "editorcore.h"
#include <QApplication>
//#include "vld.h"

int main(int argc, char *argv[])
{
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);

	editor = new EditorCore(a);
	editor->Init();

	auto ret = a.exec();

	editor->Free();
	delete editor;

	return ret;

}
