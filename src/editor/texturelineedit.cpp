#include "texturelineedit.h"
#include "ui_texturelineedit.h"
#include <QDebug>

TextureLineEdit::TextureLineEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TextureLineEdit)
{
	ui->setupUi(this);
	auto *lineEdit = ui->lineEdit;

	QObject::connect(lineEdit, &QLineEdit::returnPressed, [lineEdit, this]()
	{
		emit OnPathChanged(lineEdit->text().toLatin1());
		//qDebug() << lineEdit->text();
	});
}

void TextureLineEdit::SetPath(const char *path)
{
	ui->lineEdit->setText(path);
}

