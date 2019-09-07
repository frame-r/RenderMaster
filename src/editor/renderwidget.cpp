#include "renderwidget.h"
#include <QTime>
#include "core.h"
#include "render.h"
#include "icorerender.h"
#include "editorcore.h"
#include "gameobject.h"
#include "settings.h"
#include "editor_common.h"
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>
#include "mainwindow.h"
#include "editor_common.h"

static const float RotateSpeed = 13.0f;
static const float MoveSpeed = 6.0f;
static const float OrbitHorSpeed = 0.29f;
static const float OrbitVertSpeed = 0.20f;
static const float ZoomSpeed = 0.20f;
static const float FocusingSpeed = 10.0f;
static const int MOUSE_CLICK_MS_TRESHOLD = 200;
static QTime myTimer;

static ManagedPtr<Mesh> meshPlane;

RenderWidget::RenderWidget(QWidget *parent) :
	QWidget(parent, Qt::MSWindowsOwnDC)
{
	setMouseTracking(true);
	//installEventFilter(this);

	setAttribute(Qt::WA_NoBackground);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_PaintUnclipped);
	setAttribute(Qt::WA_NativeWindow);
	setFocusPolicy(Qt::StrongFocus);

	connect(editor, &EditorCore::OnEngineInit, this, &RenderWidget::onEngineInited, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineFree, this, &RenderWidget::onEngineFree, Qt::DirectConnection);
	connect(editor, &EditorCore::OnRender, this, &RenderWidget::onRender, Qt::DirectConnection);
	connect(editor, &EditorCore::OnUpdate, this, &RenderWidget::onUpdate, Qt::DirectConnection);
	connect(editor, &EditorCore::OnFocusOnSelected, this, &RenderWidget::onFocus, Qt::DirectConnection);
	connect(editor->window, &MainWindow::onKeyPressed, [this](Qt::Key key) { if (key == Qt::Key_Alt) keyAlt = 1; });
	connect(editor->window, &MainWindow::onKeyReleased, [this](Qt::Key key) { if (key == Qt::Key_Alt) keyAlt = 0; });

	h = (HWND)winId();

	camPos = vec3(0.0f, -10.0f, 4.0f);
	camRot = quat(70.0f, 0.0f, 0.0f);
}

void RenderWidget::onEngineInited(Core *c)
{
	core = c;
	auto *resMan = editor->core->GetResourceManager();
	meshPlane = resMan->CreateStreamMesh("std#plane");
}

void RenderWidget::onEngineFree(Core *c)
{
	meshPlane.release();
	core = nullptr;
}

void RenderWidget::engineNotLoaded()
{
	setAttribute(Qt::WA_OpaquePaintEvent, false);

	label = new QLabel(this);
	label->setAttribute(Qt::WA_TranslucentBackground);

	layout = new QHBoxLayout();
	label->setText("Engine is not loaded");
	layout->addWidget(label);
	layout->setAlignment(label, Qt::AlignmentFlag::AlignCenter);
	setLayout(layout);

	repaint();
}

void RenderWidget::engineLoaded()
{
	if (layout)
	{
		layout->removeWidget(label);
		delete label;
		label = nullptr;

		delete layout;
		layout = nullptr;
	}

	repaint();
	setAttribute(Qt::WA_OpaquePaintEvent);
}

void RenderWidget::resizeEvent(QResizeEvent* evt)
{
	Q_UNUSED( evt )
}

void RenderWidget::paintEvent(QPaintEvent* evt)
{
	Q_UNUSED( evt )
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{	
	if (event->button() == Qt::RightButton)
	{
		//qDebug() << "RenderWidget::mousePressEvent(QMouseEvent *event)";
		rightMousePressed = 1;
		oldMousePos = event->pos();
	}

	if (event->button() == Qt::LeftButton)
	{
		//qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event) (" << event->pos().x()<< event->pos().y() << ")";
		leftMouseDown = 1;
		leftMousePress = 1;
		myTimer.start();
		captureX = uint(event->pos().x());
		captureY = uint(event->pos().y());
	}
	QWidget::mousePressEvent(event);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		rightMousePressed = 0;

	if (event->button() == Qt::LeftButton)
	{
		int nMilliseconds = myTimer.elapsed();
		if (nMilliseconds < MOUSE_CLICK_MS_TRESHOLD)
			leftMouseClick = 1;

		leftMousePress = 0;
		leftMouseDown = 0;

		if (editor->currentManipulator)
			editor->currentManipulator->mouseRelease();
	}
	QWidget::mouseReleaseEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
	mousePos = event->pos();
	deltaMousePos = mousePos - oldMousePos;

	int w = size().width();
	int h = size().height();
	normalizedMousePos = vec2((float)mousePos.x() / w, (float)(h - mousePos.y()) / h);

	QWidget::mouseMoveEvent(event);
}

void RenderWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W) {keyW = 1; }
	if (event->key() == Qt::Key_S) {keyS = 1; }
	if (event->key() == Qt::Key_A) {keyA = 1; }
	if (event->key() == Qt::Key_D) {keyD = 1; }
	if (event->key() == Qt::Key_Q) {keyQ = 1; }
	if (event->key() == Qt::Key_E) {keyE = 1; }
	if (event->key() == Qt::Key_Alt) {/*qDebug() << "Widget press";*/ keyAlt = 1; }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W) {keyW = 0; }
	if (event->key() == Qt::Key_S) {keyS = 0; }
	if (event->key() == Qt::Key_A) {keyA = 0; }
	if (event->key() == Qt::Key_D) {keyD = 0; }
	if (event->key() == Qt::Key_Q) {keyQ = 0; }
	if (event->key() == Qt::Key_E) {keyE = 0; }
	if (event->key() == Qt::Key_Alt) {/*qDebug() << "Widget release";*/keyAlt = 0; }
}

bool RenderWidget::calculateCameraData(CameraData &data)
{
	if (rect().width() <= 0 || rect().height() <= 0)
		return false;

	data.pos = camPos;
	data.rot = camRot;
	data.fovInDegrees = fovInDegrees_;
	data.fovInRads = fovInDegrees_ * DEGTORAD;
	data.aspect = (float)rect().width() / rect().height();

	compositeTransform(data.WorldTransform, camPos, camRot, vec3(1.0f, 1.0f, 1.0f));

	data.ProjectionMat = perspectiveRH_ZO(data.fovInRads, data.aspect, zNear_, zFar_);

	data.ViewMat = data.WorldTransform.Inverse();
	data.ViewProjMat = data.ProjectionMat * data.ViewMat;

	data.ViewWorldDirection = -data.WorldTransform.Column3(2).Normalized();

	return true;
}

void RenderWidget::onRender()
{
	CameraData cam;
	calculateCameraData(cam);

	core->ManualRenderFrame(&h, cam.ViewMat, cam.ProjectionMat);

	ICoreRender *coreRender = core->GetCoreRender();
	Render *render = editor->core->GetRender();
	IManupulator *manipulator = editor->currentManipulator.get();
	int w = size().width();
	int h = size().height();

	if (editor->NumSelectedObjects() == 1 && manipulator)
	{
		coreRender->PushStates();
		coreRender->SetDepthTest(0);

		Settings *settings = editor->window->GetSettings();

		if (settings->isWireframeAntialiasing())
		{
			coreRender->SetMSAA(1);

			Texture *texs[1];

			coreRender->SetBlendState(BLEND_FACTOR::NONE, BLEND_FACTOR::NONE);

			Texture *msaaTex = render->GetRenderTexture(w, h, TEXTURE_FORMAT::RGBA8, 4);
			texs[0] = msaaTex;
			coreRender->SetRenderTextures(1, texs, nullptr);

			coreRender->Clear();

			manipulator->render(cam, editor->SelectionTransform(), rect());

			coreRender->SetMSAA(0);
			coreRender->SetBlendState(BLEND_FACTOR::ONE, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);

			texs[0] = coreRender->GetSurfaceColorTexture();
			coreRender->SetRenderTextures(1, texs, nullptr);

			Shader *shader;
			shader = render->GetShader("msaa_resolve.hlsl", meshPlane.get());
			coreRender->SetShader(shader);

			Texture *tex_in[] = { msaaTex };
			coreRender->BindTextures(1, tex_in, BIND_TETURE_FLAGS::PIXEL);
			coreRender->Draw(meshPlane.get(), 1);

			render->ReleaseRenderTexture(msaaTex);
		} else
		{
			Texture *texs[1];
			texs[0] = coreRender->GetSurfaceColorTexture();
			coreRender->SetRenderTextures(1, texs, nullptr);

			manipulator->render(cam, editor->SelectionTransform(), rect());
		}

		coreRender->PopStates();
	}

	if (leftMouseClick && !keyAlt)
	{
		if ((manipulator && !manipulator->isMouseIntersect(normalizedMousePos))
			|| !manipulator)
		{
			Texture *modelTex = render->GetRenderTexture(w, h, TEXTURE_FORMAT::R32UI);
			coreRender->PushStates();
			coreRender->SetDepthTest(1);

			Texture *texs[1] = { modelTex };
			coreRender->SetRenderTextures(1, texs, coreRender->GetSurfaceDepthTexture());
			coreRender->Clear();
			{
				render->DrawMeshes(PASS::ID);
			}
			texs[0] = coreRender->GetSurfaceColorTexture();
			coreRender->SetRenderTextures(1, texs, coreRender->GetSurfaceDepthTexture());

			coreRender->PopStates();


			uint32_t id;
			modelTex->ReadPixel2D(&id, captureX, captureY);
			render->ReleaseRenderTexture(modelTex);


			//qDebug() << "Captured Id = " <<  id;

			ResourceManager *resMan = editor->core->GetResourceManager();
			GameObject *ob = resMan->FindObjectById((int)id);

//			if (ob)
//				qDebug() << "found game obejct" << ob->GetName();
//			else
//				qDebug() << "game obejctnot found";

			QSet<GameObject*> slected;
			if (ob)
				slected.insert(ob);
			editor->SelectObjects(slected);

		}
	}


	coreRender->SwapBuffers();


//	ITexture *idTex;
//	render->GetRenderTexture2D(&idTex, w, h, TEXTURE_FORMAT::R32UI);
//	idTex->AddRef();

//	ITexture *depthIdTex;
//	render->GetRenderTexture2D(&depthIdTex, w, h, TEXTURE_FORMAT::D24S8);
//	depthIdTex->AddRef();

//	eng->getCoreRender()->SetDepthTest(1);

//	render->RenderPassIDPass(pCamera, idTex, depthIdTex);

//	ICoreTexture *coreTex;
//	idTex->GetCoreTexture(&coreTex);
//	uint data = 0;
//	uint read = 0;
//	eng->getCoreRender()->ReadPixel2D(coreTex, &data, &read, captureX, captureY);
//	if (read > 0)
//	{
//		qDebug() << "Captured Id = " <<  data;

//		ISceneManager *scene;
//		pCore->GetSubSystem((ISubSystem**)&scene, SUBSYSTEM_TYPE::SCENE_MANAGER);

//		IGameObject *go;
//		scene->FindChildById(&go, data);

//		if (go)
//		{
//			std::vector<RENDER_MASTER::GameObjectPtr> gos = {go};
//			editor->ChangeSelection(gos);
//		} else
//		{
//			std::vector<RENDER_MASTER::GameObjectPtr> gos;
//			editor->ChangeSelection(gos);
//		}
//	}

//	depthIdTex->Release();
//	idTex->Release();
//	render->ReleaseRenderTexture2D(idTex);
//	render->ReleaseRenderTexture2D(depthIdTex);

	leftMouseClick = 0;

}

void RenderWidget::onUpdate(float dt)
{
	CameraData cam;
	if (!calculateCameraData(cam))
		return;

	if (editor->NumSelectedObjects() == 1 && editor->currentManipulator)
	{
		if (leftMousePress)
			editor->currentManipulator->mousePress(cam, editor->SelectionTransform(), rect(), normalizedMousePos);
		else
			editor->currentManipulator->update(cam, editor->SelectionTransform(), rect(), normalizedMousePos);
	}

	if (isFocusing) // focusing
	{
		camPos = lerp(camPos, focusCameraPosition, dt * FocusingSpeed);
		if ((camPos - focusCameraPosition).Lenght() < 0.01f)
			isFocusing = 0;
	}
	else if (keyAlt) // orbit mode
	{
		vec3 dist = camPos - focusWorldCenter;
		Spherical posSpherical = ToSpherical(dist);

		if (rightMousePressed) // zoom
		{
			posSpherical.r += (-deltaMousePos.x() - deltaMousePos.y()) * posSpherical.r * dt * ZoomSpeed;
			posSpherical.r = clamp(posSpherical.r, 0.01f, std::numeric_limits<float>::max());
			camPos = ToCartesian(posSpherical) + focusWorldCenter;
		}

		if (leftMouseDown) // orbit
		{
			posSpherical.phi -= deltaMousePos.x() * dt * OrbitHorSpeed;
			posSpherical.theta -= deltaMousePos.y() * dt * OrbitVertSpeed;
			camPos = ToCartesian(posSpherical) + focusWorldCenter;

			mat4 newRotMat;
			lookAtCamera(newRotMat, camPos, focusWorldCenter);
			newRotMat.Transpose();
			camRot = quat(newRotMat);
		}
	}
	else if (rightMousePressed) // free moving
	{
		vec3 orthDirection = vec3(cam.WorldTransform.Column(0)); // X local
		vec3 forwardDirection = -vec3(cam.WorldTransform.Column(2)); // -Z local
		vec3 upDirection = vec3(0.0f, 0.0f, 1.0f); // Z world

		if (keyA)
			camPos -= orthDirection * dt * MoveSpeed;

		if (keyD)
			camPos += orthDirection * dt * MoveSpeed;

		if (keyW)
			camPos += forwardDirection * dt * MoveSpeed;

		if (keyS)
			camPos -= forwardDirection * dt * MoveSpeed;

		if (keyQ)
			camPos -= upDirection * dt * MoveSpeed;

		if (keyE)
			camPos += upDirection * dt * MoveSpeed;

		quat dxRot = quat(-deltaMousePos.y() * dt * RotateSpeed, 0.0f, 0.0f);
		quat dyRot = quat(0.0f, 0.0f,-deltaMousePos.x() * dt * RotateSpeed);
		camRot = dyRot * camRot * dxRot;

		// also update focus center
		focusWorldCenter = camPos + forwardDirection * focusRadius;
	}

	oldMousePos = mousePos;
	deltaMousePos = {};
	leftMousePress = 0;
}

void RenderWidget::onFocus(const vec3 &worldCenter)
{
	CameraData cam;
	if (!calculateCameraData(cam))
		return;

	isFocusing = 1;

	focusWorldCenter = worldCenter;

	float distance = 1.5f / tan(cam.fovInRads * 0.5f);
	focusCameraPosition = worldCenter - cam.ViewWorldDirection * distance;

	vec3 dist = camPos - focusWorldCenter;
	Spherical posSpherical = ToSpherical(dist);
	focusRadius = posSpherical.r;
}
