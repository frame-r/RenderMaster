#ifndef MANIPULATORROTATOR_H
#define MANIPULATORROTATOR_H
#include "imanipulator.h"

class ManipulatorRotator : public IManupulator
{

public:
	ManipulatorRotator();
	virtual ~ManipulatorRotator();

	void render(const CameraData& cam, const mat4 selectionTransform, const QRect& screen) override;
	void update(const CameraData& cam, const mat4 selectionTransform, const QRect& screen, const vec2 &normalizedMousePos) override;
	bool isMouseIntersect(const vec2 &normalizedMousePos) override;
	void mousePress(const CameraData& cam, const mat4 selectionTransform, const QRect &screen, const vec2 &normalizedMousePos) override;
	void mouseRelease() override;

};

#endif // MANIPULATORROTATOR_H
