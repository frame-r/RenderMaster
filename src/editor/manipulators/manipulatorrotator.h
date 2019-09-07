#ifndef MANIPULATORROTATOR_H
#define MANIPULATORROTATOR_H
#include "imanipulator.h"

class ManipulatorRotator : public IManupulator
{
	enum class ELEMENT
	{
		NONE = 0,
		INTERSECTION_X,
		INTERSECTION_Y,
		INTERSECTION_Z,
	};
	ELEMENT underMouse{};

	enum class STATE
	{
		NONE,
		ROTATING_X,
		ROTATING_Y,
		ROTATING_Z,
	};
	STATE state_{};

	float startAngle;
	Plane rotatePlane;
	quat startQuat;
	mat4 startInvSelectionWorldTransform;

public:
	ManipulatorRotator();
	virtual ~ManipulatorRotator();

	void render(const CameraData& cam, const mat4& selectionTransform, const QRect& screen) override;
	void update(const CameraData& cam, const mat4& selectionTransform, const QRect& screen, const vec2 &normalizedMousePos) override;
	bool isMouseIntersect(const vec2 &normalizedMousePos) override;
	void mousePress(const CameraData& cam, const mat4& selectionTransform, const QRect &screen, const vec2 &normalizedMousePos) override;
	void mouseRelease() override;

};

#endif // MANIPULATORROTATOR_H
