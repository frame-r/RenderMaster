#ifndef D3D11WIDGET_H
#define D3D11WIDGET_H
#include <QWidget>
#include <QPoint>
#include <vector_math.h>
#include "editor_common.h"

class QLabel;
class QHBoxLayout;
class Core;
class ICoreRender;


class RenderWidget : public QWidget
{
    Q_OBJECT

	// Viewport Camera
	vec3 camPos;
	quat camRot;
	float zNear_ = 0.10f;
	float zFar_ = 1000.f;
	float fovInDegrees_ = 60.0f;

	// Camera focusing data
	int isFocusing = 0;
	vec3 focusWorldCenter;
	vec3 focusCameraPosition;
	float focusRadius = 1.0f;

	// Key & mouse state
	int rightMousePressed = 0;
	int leftMouseDown = 0; // live while mouse button is down
	int leftMousePress = 0; // live one frame after mouse button press
	int leftMouseClick = 0; // live one frame after mouse button released
	QPoint mousePos;
	QPoint oldMousePos;
	QPoint deltaMousePos;
	vec2 normalizedMousePos;
	int keyW = 0;
	int keyS = 0;
	int keyA = 0;
	int keyD = 0;
	int keyQ = 0;
	int keyE = 0;
	int keyAlt = 0;

	// Picking
	uint captureX = 0, captureY = 0;

	HWND h;

	Core *core = nullptr;

public:
	explicit RenderWidget(QWidget *parent = 0);

	QPaintEngine* paintEngine() const override { return NULL; }

protected:
	void resizeEvent(QResizeEvent* evt) override;
	void paintEvent(QPaintEvent* evt) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;

private slots:
	void onEngineInited(Core *c);
	void onEngineFree(Core *c);
	void onRender();
	void onUpdate(float dt);
	void onFocus(const vec3& center);

private:
	void engineNotLoaded();
	void engineLoaded();
	void calculateCameraData(CameraData& data);

	QLabel *label{};
	QHBoxLayout *layout{};

};

#endif // D3D11WIDGET_H
