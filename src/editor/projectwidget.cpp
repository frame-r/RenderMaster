#include "projectwidget.h"
#include "ui_projectwidget.h"
#include <QFileSystemModel>
#include <QFileDialog>
#include "editorcore.h"
#include "resource_manager.h"
#include "core.h"


class MyFileSystemModel : public QFileSystemModel {
public:
	QVariant data(const QModelIndex &index, int role) const
	{
		if( role == Qt::DecorationRole && index.column() == 0)
			{
				QFileInfo info = MyFileSystemModel::fileInfo(index);

				if(info.isFile())
				{
					if(info.suffix() == "dds" || info.suffix() == "png" || info.suffix() == "jpg" || info.suffix() == "jpeg")
						return QPixmap(":/icons/img.png");
					else if (info.suffix() == "mat" || info.suffix() == "genericmat")
						return  QPixmap(":/icons/material.png");
					else if (info.suffix() == "fbx" || info.suffix() == "mesh")
						return  QPixmap(":/icons/mesh.png");
					else
						return  QPixmap(":/icons/bin.png");
				}
			}
		return QFileSystemModel::data(index, role);
	}
};

ProjectWidget::ProjectWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ProjectWidget)
{
	ui->setupUi(this);

	model = new MyFileSystemModel;
	model->setRootPath(QDir::currentPath());

	QTreeView *view = ui->treeView;
	view->setModel(model);
	view->setHeaderHidden(true);

	for (int i = 1; i < model->columnCount(); ++i)
		view->hideColumn(i);

	connect(editor, &EditorCore::OnEngineInit, this, &ProjectWidget::onEngineInited, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineFree, this, &ProjectWidget::onEngineFree, Qt::DirectConnection);

}

void ProjectWidget::onEngineInited(Core *c)
{
	std::string path = c->GetDataPath();
	QDir dir(path.c_str());
	if (!dir.exists())
		qWarning("Dir doesn't exist");

	model->setRootPath(dir.absolutePath());

	ui->treeView->setRootIndex(model->index(path.c_str()));
}

void ProjectWidget::onEngineFree(Core *c)
{
}

ProjectWidget::~ProjectWidget()
{
	delete model;
	delete ui;
}


void ProjectWidget::on_pushButton_clicked()
{
	if (!editor->IsEngineLoaded())
		return;

	std::string p = editor->core->GetDataPath();
	QString	path = QString(p.c_str());
	QString file1Name = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("All files (*.*);;JPEG (*.jpg *.jpeg);;FBX Files (*.fbx)"));

	auto *resMan = editor->core->GetResourceManager();
	resMan->Import(file1Name.toLatin1());
}
