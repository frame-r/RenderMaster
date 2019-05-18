#ifndef COMMON_H
#define COMMON_H
#undef min
#undef max

#include <vector_math.h>
#include <QString>

struct CameraData
{
	vec3 pos;
	quat rot;
	float fovInDegrees;
	float fovInRads;
	float aspect;
	mat4 WorldTransform;
	mat4 ViewMat;
	mat4 ProjectionMat;
	mat4 ViewProjMat;
	vec3 ViewWorldDirection;
};


QString vec3ToString(const vec3& v);
QString quatToString(const quat& v);
QString vec2ToString(const vec2& v);
float clamp(float f);

template<typename T>
T lerp(const T& l, const T& r, float v)
{
	v = clamp(v);
	return l * (1.0f - v) + r * v;
}

inline float clamp(float n, float lower, float upper)
{
	return std::max(lower, std::min(n, upper));
}

struct Spherical
{
	float r;
	float theta;
	float phi;
};

QString sphericalToString(const Spherical& v);

Spherical ToSpherical(const vec3& pos);
vec3 ToCartesian(const Spherical& pos);

void lookAtCamera(mat4& Result, const vec3 &eye, const vec3 &center);


struct Plane
{
	vec3 origin;
	vec3 normal;

	Plane(const vec3& normalIn, const vec3& originIn) :
		origin(originIn), normal(normalIn){}
};


struct Line3D
{
	vec3 origin;
	vec3 direction;

	Line3D() = default;
	Line3D(const vec3& directionlIn, const vec3& originIn) :
		origin(originIn), direction(directionlIn){}

	vec3 projectPoint(vec3 &worldPos);
};

class QLabel;
void setLabel(QLabel *l, float val);


bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line);
Line3D MouseToRay(const mat4& cameraModelMatrix, float fov, float aspect, const vec2& normalizedMousePos);
vec2 WorldToNdc(const vec3& pos, const mat4& ViewProj);
float PointToSegmentDistance(const vec2& p0, const vec2& p1, const vec2& ndc);
vec2 NdcToScreen(const vec2& pos, uint w, uint h);
float DistanceTo(const mat4& ViewProj, const mat4& worldTransform);
#endif // COMMON_H
