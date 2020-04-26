#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include "imanipulator.h"

class ManipulatorTranslator : public IManupulator
{
	enum class STATE
	{
		NONE,
		MOVING_ARROW_HANDLE,
		MOVING_PLANE_HANDLE
	};

	MANIPULATOR_ELEMENT underMouse = MANIPULATOR_ELEMENT::NONE;
	STATE state = STATE::NONE;
	vec3 worldDelta;
	vec2 oldNormalizedMousePos;
	Ray movesAlongLine;
	Plane movesAlongPlane;

public:
	ManipulatorTranslator();
	virtual ~ManipulatorTranslator();

	void render(const CameraData& cam, const mat4& selectionTransform, const QRect& screen) override;
	void updateMouse(const CameraData& cam, const mat4& selectionTransform, const QRect& screen, const vec2& normalizedMousePos) override;
	bool isMouseIntersect(const vec2 &normalizedMousePos) override;
	void mousePress(const CameraData& cam, const mat4& selectionTransform, const QRect &screen, const vec2& normalizedMousePos) override;
	void mouseRelease() override;
};

#endif // MANIPULATORTRANSLATOR_H
