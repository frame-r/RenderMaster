#include "texturelineedit.h"
#include "ui_texturelineedit.h"
#include "editor_common.h"

TextureLineEdit::TextureLineEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TextureLineEdit)
{
	ui->setupUi(this);
	auto *lineEdit = ui->lineEdit;

	QObject::connect(lineEdit, &QLineEdit::returnPressed, [lineEdit, this]()
	{
		emit OnPathChanged(lineEdit->text().toLatin1());
		uvChanged();
	});

	QObject::connect(lineEdit, &MyLineEdit::fileDropped, [this, lineEdit](QString relativePath)
	{
		if (isTexture(relativePath))
		{
			lineEdit->setText(relativePath);
			emit OnPathChanged(relativePath.toLatin1());
		}
	});

	QObject::connect(ui->uv1_sb, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), this, &TextureLineEdit::uvChanged);
	QObject::connect(ui->uv2_sb, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), this, &TextureLineEdit::uvChanged);
	QObject::connect(ui->uv3_sb, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), this, &TextureLineEdit::uvChanged);
	QObject::connect(ui->uv4_sb, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged), this, &TextureLineEdit::uvChanged);
}

TextureLineEdit::~TextureLineEdit()
{
	delete ui;
}
void TextureLineEdit::SetPath(const char *path)
{
	ui->lineEdit->setText(path);
}

void TextureLineEdit::SetUV(const vec4 &uv)
{
	ui->uv1_sb->setValue(uv.x);
	ui->uv2_sb->setValue(uv.y);
	ui->uv3_sb->setValue(uv.z);
	ui->uv4_sb->setValue(uv.w);
}

void TextureLineEdit::uvChanged()
{
	vec4 uv;
	uv.x = ui->uv1_sb->value();
	uv.y = ui->uv2_sb->value();
	uv.z = ui->uv3_sb->value();
	uv.w = ui->uv4_sb->value();
	emit OnUVChanged(uv);
}

