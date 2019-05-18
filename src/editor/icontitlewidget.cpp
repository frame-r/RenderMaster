#include "icontitlewidget.h"

#include <QIcon>
#include <QString>
#include <QBoxLayout>
#include <QLabel>
#include <QStyle>

IconTitleWidget::IconTitleWidget(const QIcon& icon, const QString& title, QWidget *parent) :
    QFrame(parent)
{
	Q_UNUSED( icon )

//	QPalette Pal(palette());
//	Pal.setColor(QPalette::Background, Qt::black);
//	setAutoFillBackground(true);
//	setPalette(Pal);

	QBoxLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
	l->setContentsMargins(0, 0, 0, 0);
	setLayout(l);

//    _iconLabel = new QLabel();
//    l->addWidget(_iconLabel);

	_titleLabel = new QLabel();
	l->addWidget(_titleLabel, 1);

    //setIcon(icon);
	_titleLabel->setText(title);
	_titleLabel->setVisible(true);

}

//void IconTitleWidget::setIcon(const QIcon& icon)
//{
//    if (icon.isNull())
//    {
//        _iconLabel->setPixmap(QPixmap());
//        _iconLabel->setVisible(false);
//    }
//    else
//    {
//		_iconLabel->setPixmap(icon.pixmap(8, 8));
//        _iconLabel->setVisible(true);
//    }
//}

//void IconTitleWidget::setTitle(const QString& title)
//{
//    if (title.isEmpty())
//    {
//        _titleLabel->setText(QString());
//        _titleLabel->setVisible(false);
//    }
//    else
//    {
//        _titleLabel->setText(title);
//        _titleLabel->setVisible(true);
//    }
//}

//void IconTitleWidget::polishUpdate()
//{
//    QList<QWidget*> widgets;
//    widgets.append(_iconLabel);
//    widgets.append(_titleLabel);
//    foreach (QWidget* w, widgets)
//    {
//        w->style()->unpolish(w);
//        w->style()->polish(w);
//        w->update();
//    }
//}
