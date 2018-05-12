#pragma once
#include "Common.h"

#define IGAMEOBJECT_IMPLEMENTATION \
private: \
\
vec3 _pos{ 0.0f, 0.0f, 0.0f }; \
vec3 _rot{ 0.0f, 0.0f, 0.0f }; \
vec3 _scale{ 1.0f, 1.0f, 1.0f }; \
\
public: \
\
API SetPosition(const vec3 *pos) override \
{ \
	_pos = *pos; \
	return S_OK; \
} \
API SetRotation(const vec3 *rot) override \
{ \
	_rot = *rot; \
	_rot.x = fmod(_rot.x, 360.0f); \
	_rot.y = fmod(_rot.y, 360.0f); \
	_rot.z = fmod(_rot.z, 360.0f); \
	return S_OK; \
} \
\
API GetModelMatrix(OUT mat4 *mat) override \
{ \
	mat4 R; \
	mat4 T; \
	mat4 S; \
\
	const float DEG2RAD = 3.141593f / 180; \
	float sx, sy, sz, cx, cy, cz, theta; \
\
	theta = _rot.x * DEG2RAD; \
	sx = sinf(theta); \
	cx = cosf(theta); \
\
	theta = _rot.y * DEG2RAD; \
	sy = sinf(theta); \
	cy = cosf(theta); \
\
	theta = _rot.z * DEG2RAD; \
	sz = sinf(theta); \
	cz = cosf(theta); \
\
	R.el_2D[0][0] = cy * cz; \
	R.el_2D[1][0] = sx*sy*cz + cx*sz; \
	R.el_2D[2][0] = -cx*sy*cz + sx*sz; \
	R.el_2D[0][1] = -cy*sz; \
	R.el_2D[1][1] = -sx*sy*sz + cx*cz; \
	R.el_2D[2][1] = cx*sy*sz + sx*cz; \
	R.el_2D[0][2] = sy; \
	R.el_2D[1][2] = -sx*cy; \
	R.el_2D[2][2] = cx*cy; \
\
	T.el_2D[0][3] = _pos.x; \
	T.el_2D[1][3] = _pos.y; \
	T.el_2D[2][3] = _pos.z; \
\
	S.el_2D[0][0] = _scale.x; \
	S.el_2D[1][1] = _scale.y; \
	S.el_2D[2][2] = _scale.z; \
\
	*mat =  T * R * S; \
\
	return S_OK; \
}


class GameObject : public IGameObject
{
	IGAMEOBJECT_IMPLEMENTATION
};
