#include "texturelineedit.h"
#include "ui_texturelineedit.h"
#include <QDebug>

TextureLineEdit::TextureLineEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TextureLineEdit)
{
	ui->setupUi(this);
	lineEdit = ui->lineEdit;
}

TextureLineEdit::~TextureLineEdit()
{
	delete ui;
}

void TextureLineEdit::SetPath(const char *path)
{
	if (strlen(path)>0)
		ui->lineEdit->setText(QString(path));
	else
		qDebug() << "empty path";
}

//template<typename T>
//TextureLineEditConcrete<T>::TextureLineEditConcrete(QWidget *parent, TextureSetCallback<T> fnSet, TextureGetCallback<T> fnGet)
//	: TextureLineEdit(parent)
//{
//	fnSet_ = fnSet;
//	fnGet_ = fnGet;
//	//connect(ui->lineEdit, &QLineEdit::returnPressed, []() {});

//}
