#include "selectmeshdialog.h"
#include "ui_selectmeshdialog.h"
#include "editorcore.h"
#include "resource_manager.h"



SelectMeshDialog::SelectMeshDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SelectMeshDialog)
{
	ui->setupUi(this);

	QObject::connect(ui->listWidget, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( onListDoubleClicked( QModelIndex ) ));
	QObject::connect(ui->listWidget, SIGNAL( clicked( QModelIndex ) ), this, SLOT( onListClicked( QModelIndex ) ));

	std::vector<std::string> meshes = editor->resMan->GetImportedMeshes();

	for(std::string &i : meshes)
	{
		ui->listWidget->addItem(QString(i.c_str()));
	}
}

SelectMeshDialog::~SelectMeshDialog()
{
	delete ui;
}

void SelectMeshDialog::onListDoubleClicked(const QModelIndex &index)
{
	if( !index.isValid() ) {
		return;
	}

	mesh = index.data().toString();
	accept();
}

void SelectMeshDialog::onListClicked(const QModelIndex &index)
{
	if( !index.isValid() ) {
		return;
	}

	mesh = index.data().toString();
}
