#include "imanipulator.h"

vec4 AxesColors[3] = {{1,0,0,1},{0,1,0,1},{0,0,1,1}};
extern const char* PrimitiveShaderName = "primitive.hlsl";
vec3 AxesEndpoints[3] = {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};
float MaxDistanceInPixels = 1000000.0f;
float SelectionThresholdInPixels = 8.0f;

IManupulator::IManupulator(QObject *parent) : QObject(parent)
{
}

AxisIntersection intersectMouseWithAxis(const CameraData& cam, const mat4 selectionWS, const QRect &screen, const vec2 &normalizedMousePos, const vec3 &axisDirWS, MANIPULATOR_ELEMENT type)
{
	AxisIntersection out;

	vec3 center = selectionWS.Column3(3);
	vec4 center4 = vec4(selectionWS.Column3(3));

	vec3 V = (center - cam.pos).Normalized();
	vec3 VcrossAxis = V.Cross(axisDirWS).Normalized();
	vec3 N = axisDirWS.Cross(VcrossAxis).Normalized();

	Plane plane(N, center);

	Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

	if (RayPlaneIntersection(out.worldPos, plane, ray))
	{
	   vec2 A = NdcToScreen(WorldToNdc(center, cam.ViewProjMat), screen);

	   vec4 axisEndpointLocal = vec4(AxesEndpoints[(int)type] * axisScale(center4, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));
	   vec4 axisEndpointWorld = selectionWS * axisEndpointLocal;
	   vec2 B = NdcToScreen(WorldToNdc(vec3(axisEndpointWorld), cam.ViewProjMat), screen);

	   vec2 I = NdcToScreen(WorldToNdc(out.worldPos, cam.ViewProjMat), screen);

	   out.minDistToAxes = PointToSegmentDistance(A, B, I);
	   return out;
	}

	out.minDistToAxes = MaxDistanceInPixels;

	return out;
}
