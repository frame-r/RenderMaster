#ifndef TEXTURELINEEDIT_H
#define TEXTURELINEEDIT_H

#include <QWidget>
#include <QLineEdit>

template<typename T>
using TextureSetCallback = void (T::*)(const char *);

template<typename T>
using TextureGetCallback = const char* (T::*)();

namespace Ui {
class TextureLineEdit;
}

class TextureLineEdit : public QWidget
{
	Q_OBJECT

public:
	explicit TextureLineEdit(QWidget *parent = nullptr);
	~TextureLineEdit();

protected:
	void SetPath(const char *path);

	QLineEdit *lineEdit;
private:
	Ui::TextureLineEdit *ui;
};


template<class T>
class TextureLineEditConcrete : public TextureLineEdit
{
	T *obj_;
	TextureSetCallback<T> fnSet_;
	TextureGetCallback<T> fnGet_;

	void OnPathChanged();

public:
	explicit TextureLineEditConcrete(QWidget *parent, TextureSetCallback<T> fnSet, TextureGetCallback<T> fnGet)
		: TextureLineEdit(parent)
	{
		fnSet_ = fnSet;
		fnGet_ = fnGet;
		QObject::connect(lineEdit, &QLineEdit::returnPressed, [this]()
		{
			if (obj_)
				(*obj_.*fnSet_)(lineEdit->text().toLatin1());
		});
	}

	void SetObject(T* obj)
	{
		obj_ = obj;

		if (obj_)
		{
			SetPath((*obj_.*fnGet_)());
		}
	}
};

#endif // TEXTURELINEEDIT_H
