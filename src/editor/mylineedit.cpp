#include "MyLineEdit.h"
#include <QDropEvent>
#include <QDir>
#include <QMimeData>
#include <qdebug.h>
#include "editorcore.h"
#include "core.h"

void MyLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
	  e->acceptProposedAction();
}
void MyLineEdit::dropEvent(QDropEvent *e)
{

  const QMimeData *d = e->mimeData();
  QString path = d->text();
  if (path.startsWith("file:///"))
  {
		Core *core = editor->core;
		if (core)
		{
			std::string dataPath = core->GetDataPath();

			QString absolutePath = path.mid(8);

			QDir dataPathDir(dataPath.c_str());
			QString relativePath = dataPathDir.relativeFilePath(absolutePath);

			emit fileDropped(relativePath);

			qDebug() << relativePath;
		}
  }
}

