#ifndef SELECTMESHDIALOG_H
#define SELECTMESHDIALOG_H

#include <QDialog>

namespace Ui {
class SelectMeshDialog;
}

class SelectMeshDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SelectMeshDialog(QWidget *parent = nullptr);
	~SelectMeshDialog();

	QString mesh;

private:
	Ui::SelectMeshDialog *ui;

private slots:
	void onListDoubleClicked( const QModelIndex& index );
	void onListClicked( const QModelIndex& index );
};

#endif // SELECTMESHDIALOG_H
