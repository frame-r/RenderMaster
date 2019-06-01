#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include "imanipulator.h"

enum class MANIPULATOR_ELEMENT
{
	NONE = -1,
	X,
	Y,
	Z,
	XY,
	YZ,
	ZX
};

enum MANIPULATOR_STATE
{
	NONE,
	MOVING_ARROW_HANDLE,
	MOVING_PLANE_HANDLE
};

class ManipulatorTranslator : public IManupulator
{
	MANIPULATOR_ELEMENT underMouse = MANIPULATOR_ELEMENT::NONE;
	MANIPULATOR_STATE state = MANIPULATOR_STATE::NONE;
	vec3 worldDelta;
	vec2 oldNormalizedMousePos;
	Ray movesAlongLine;
	Plane movesAlongPlane;

public:
	ManipulatorTranslator();
	virtual ~ManipulatorTranslator();

	void render(const CameraData& cam, const mat4 selectionTransform, const QRect& screen) override;
	void update(const CameraData& cam, const mat4 selectionTransform, const QRect& screen, const vec2 &normalizedMousePos) override;
	bool isMouseIntersect(const vec2 &normalizedMousePos) override;
	void mousePress(const CameraData& cam, const mat4 selectionTransform, const QRect &screen, const vec2 &normalizedMousePos) override;
	void mouseRelease() override;
};

#endif // MANIPULATORTRANSLATOR_H
