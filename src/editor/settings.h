#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>

class Core;

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
	Q_OBJECT

	bool wireframeAntialiasing{true};

public:
	explicit Settings(QWidget *parent = nullptr);
	~Settings();

	bool isWireframeAntialiasing() const { return wireframeAntialiasing; }

private:
	Ui::Settings *ui;
private slots:
	void OnEngineFree(Core *c);
	void OnEngineInit(Core *c);
	void sliderOChanged(int i);
	void slider1Changed(int i);
};

#endif // SETTINGS_H
