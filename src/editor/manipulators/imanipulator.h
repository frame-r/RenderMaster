#ifndef MANIPULATORABSTRACT_H
#define MANIPULATORABSTRACT_H

#include <QObject>
#include "vector_math.h"
#include "../editor_common.h"

class IManupulator : public QObject
{
	Q_OBJECT

public:
	explicit IManupulator(QObject *parent = nullptr);
	virtual ~IManupulator() {}

	void virtual render(const CameraData& cam, const mat4 selectionTransform, const QRect& screen) = 0;
	void virtual update(const CameraData& cam, const mat4 selectionTransform, const QRect& screen, const vec2 &normalizedMousePos) = 0;
	bool virtual isMouseIntersect(const vec2 &normalizedMousePos) = 0;
	void virtual mousePress(const CameraData& cam, const mat4 selectionTransform,const QRect &screen, const vec2 &normalizedMousePos) = 0;
	void virtual mouseRelease() = 0;
};

extern vec4 AxesColors[3];

#endif // MANIPULATORABSTRACT_H
