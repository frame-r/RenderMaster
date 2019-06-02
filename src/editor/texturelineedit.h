#ifndef TEXTURELINEEDIT_H
#define TEXTURELINEEDIT_H

#include <QWidget>
#include <QLineEdit>


namespace Ui {
class TextureLineEdit;
}

class TextureLineEdit : public QWidget
{
	Q_OBJECT

public:
	explicit TextureLineEdit(QWidget *parent = nullptr);
	//~TextureLineEdit();

signals:
	void OnPathChanged(const char *path);

//protected:
//	void SetPath(const char *path);

private:
	Ui::TextureLineEdit *ui;
};


#endif // TEXTURELINEEDIT_H
