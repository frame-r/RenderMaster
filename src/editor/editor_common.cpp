#include "editor_common.h"
#include "common.h"
#include <algorithm>
#include <QLabel>

QString vec3ToString(const vec3& v)
{
	return QString('{') +
			QString::number(v.x) + QString(", ") +
			QString::number(v.y) + QString(", ") +
			QString::number(v.z) + QString('}');
}

QString vec2ToString(const vec2& v)
{
	return QString('{') +
			QString::number(v.x) + QString(", ") +
			QString::number(v.y) + QString('}');
}

float clamp(float f)
{
	return std::min(1.0f, std::max(0.0f, f));
}

Spherical ToSpherical(const vec3& pos)
{
	float r = sqrt(pos.Dot(pos));
	float theta = acos(pos.z / r);
	float phi = atan2(pos.y, pos.x);

	return Spherical{r, theta, phi};
}

vec3 ToCartesian(const Spherical &pos)
{
	return vec3(
				pos.r * sin(pos.theta) * cos(pos.phi),
				pos.r * sin(pos.theta) * sin(pos.phi),
				pos.r * cos(pos.theta));
}


QString sphericalToString(const Spherical &v)
{
	return QString("{r: ") +
			QString::number(v.r) + QString(", theta: ") +
			QString::number(v.theta) + QString(", phi: ") +
			QString::number(v.phi) + QString('}');
}

float axisScale(const vec4& worldPos, const mat4& View, const mat4& Proj, const QPoint& screenSize)
{
	vec4 p0 = Proj * View * worldPos;
	vec4 xx = vec4(View.el_2D[0][0], View.el_2D[0][1], View.el_2D[0][2], 0.0f);
	vec4 p1 = Proj * View * (worldPos + xx);
	p0 = p0 / abs(p0.w);
	p1 = p1 / abs(p1.w);

	float x = (p1.x - p0.x) * 0.5f * screenSize.x();
	float y = (p1.y - p0.y) * 0.5f * screenSize.y();

	return float(120.0f / (vec3(x, y, 0.0f).Lenght()));
}

void lookAtCamera(mat4& Result, const vec3 &eye, const vec3 &center)
{
	Result = mat4(1.0f);
	vec3 Z = (eye - center).Normalize();
	vec3 X = vec3(0.0f, 0.0f, 1.0f).Cross(Z).Normalize();
	vec3 Y(Z.Cross(X));
	Y.Normalize();
	Result.el_2D[0][0] = X.x;
	Result.el_2D[0][1] = X.y;
	Result.el_2D[0][2] = X.z;
	Result.el_2D[1][0] = Y.x;
	Result.el_2D[1][1] = Y.y;
	Result.el_2D[1][2] = Y.z;
	Result.el_2D[2][0] = Z.x;
	Result.el_2D[2][1] = Z.y;
	Result.el_2D[2][2] = Z.z;
	Result.el_2D[0][3] = -X.Dot(eye);
	Result.el_2D[1][3] = -Y.Dot(eye);
	Result.el_2D[2][3] = -Z.Dot(eye);
}

QString quatToString(const quat &q)
{
	return QString("{x: ") +
			QString::number(q.x) + QString(", y: ") +
			QString::number(q.y) + QString(", z: ") +
			QString::number(q.z) + QString(", w: ") +
			QString::number(q.w) + QString('}');
}

bool LineIntersectPlane(vec3& intersection, const Plane& plane, const Line3D& line)
{
	vec3 R = line.direction.Normalized();
	vec3 N = plane.normal.Normalized();

	float d = N.Dot(plane.origin);

	float denom = N.Dot(R);
	if (abs(denom) < 0.00001f) return false;

	float x = (d - N.Dot(line.origin)) / denom;

	intersection = line.origin + R * x;

	return true;
}

Line3D MouseToRay(const mat4& cameraModelMatrix, float fovInDegree, float aspect, const vec2& ndc)
{
	vec3 forwardN = -cameraModelMatrix.Column3(2).Normalized();

	float y = tan(DEGTORAD * fovInDegree * 0.5f);
	float x = y;

	vec3 rightN = cameraModelMatrix.Column3(0).Normalized();
	vec3 right = rightN * x * aspect;

	vec3 upN = cameraModelMatrix.Column3(1).Normalized();
	vec3 up = upN * y;

	vec2 mousePos = ndc * 2.0f - vec2(1.0f, 1.0f);

	vec3 dir = (forwardN + right * mousePos.x + up * mousePos.y).Normalized();
	vec3 origin = cameraModelMatrix.Column3(3);

	return Line3D(dir, origin);
}

vec2 WorldToNdc(const vec3& pos, const mat4& ViewProj)
{
	vec4 screenPos = ViewProj * vec4(pos);
	screenPos /= screenPos.w;
	return vec2(screenPos.x, screenPos.y);
}

float PointToSegmentDistance(const vec2& p0, const vec2& p1, const vec2& point)
{
	vec2 direction = vec2(p1.x - p0.x, p1.y - p0.y);
	vec2 p00 = p0;
	return (p00 + direction * clamp(direction.Dot(point - p00) / direction.Dot(direction), 0.0f, 1.0f) - point).Lenght();
}

vec2 NdcToScreen(const vec2 &ndc, uint w, uint h)
{
	vec2 tmp = ndc * 0.5f + vec2(0.5f, 0.5f);
	return vec2(tmp.x * w, tmp.y * h);
}

float DistanceTo(const mat4& ViewProj, const mat4& worldTransform)
{
	vec4 view4 = ViewProj * vec4(worldTransform.el_2D[0][3], worldTransform.el_2D[1][3], worldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	return view.Lenght();
}

vec3 Line3D::projectPoint(vec3 &worldPos)
{
	vec3 AP = worldPos - origin;
	vec3 AB = direction;
	return origin + direction * AP.Dot(AB);
}

void setLabel(QLabel *l, float val)
{
	QString text;
	text.sprintf("%6.2f", val);
	l->setText(text);
}
