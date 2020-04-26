#ifndef MANIPULATORABSTRACT_H
#define MANIPULATORABSTRACT_H

#include <QObject>
#include "vector_math.h"
#include "../editor_common.h"

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

inline bool isAxisMovement(MANIPULATOR_ELEMENT e)
{
	return MANIPULATOR_ELEMENT::NONE < e && e <= MANIPULATOR_ELEMENT::Z;
}

inline bool isPlaneMovement(MANIPULATOR_ELEMENT e)
{
	return MANIPULATOR_ELEMENT::XY <= e && e <= MANIPULATOR_ELEMENT::ZX;
}

class IManupulator : public QObject
{
	Q_OBJECT

public:
	explicit IManupulator(QObject *parent = nullptr);
	virtual ~IManupulator() = default;

	void virtual render(const CameraData& cam, const mat4& selectionTransform, const QRect& screen) = 0;
	void virtual updateMouse(const CameraData& cam, const mat4& selectionTransform, const QRect& screen, const vec2 &normalizedMousePos) = 0;
	bool virtual isMouseIntersect(const vec2 &normalizedMousePos) = 0;
	void virtual mousePress(const CameraData& cam, const mat4& selectionTransform,const QRect &screen, const vec2 &normalizedMousePos) = 0;
	void virtual mouseRelease() = 0;
};

extern float SelectionThresholdInPixels;
extern vec4 AxesColors[3];
extern vec4 ColorYellow;
extern vec4 ColorTransparent;
extern const char* PrimitiveShaderName;
extern vec3 AxesEndpoints[3];
extern float MaxDistanceInPixels;
extern float SelectionThresholdInPixels;

struct AxisIntersection
{
	float minDistToAxes;
	vec3 worldPos;
};

AxisIntersection intersectMouseWithAxis(const CameraData& cam,
										const mat4 selectionWS,
										const QRect &screen,
										const vec2 &normalizedMousePos,
										const vec3 &axisDirWS,
										MANIPULATOR_ELEMENT type);

#endif // MANIPULATORABSTRACT_H
