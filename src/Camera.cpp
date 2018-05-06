#include "Camera.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Camera::Camera()
{
}

API Camera::GetViewProjectionMatrix(mat4& mat, float aspect)
{
	mat4 P;
	mat4 V;

	const float DEGTORAD = 3.1415926f / 180.0f;
	float const tanHalfFovy = tan(DEGTORAD* _fovAngle / 2);
	P.el_2D[0][0] = 1.0f / (aspect * tanHalfFovy);
	P.el_2D[1][1] = 1.0f / (tanHalfFovy);
	P.el_2D[2][2] = (_zFar + _zNear) / (_zFar - _zNear);
	P.el_2D[2][3] = 1.0f;
	P.el_2D[3][2] = -(2.0f * _zFar * _zNear) / (_zFar - _zNear);
	P.el_2D[3][3] = 0.0f;

	//V.el_2D[0][3] = -20.0f;
	//V.el_2D[1][3] = -20.0f;

	vec3 position(-9.5f, 9.5f, -12.5f);
	vec3 forward = position.Normalized() * -1.0f;

	look_at(V, position, position + forward);

	mat = V*P;

	vec4 ze(0.0f, 0.0f, 0.0f, 1.0f);
	ze = mat * ze;
	ze /= ze.w;

	vec4 x(1.0f, 0.0f, 0.0f, 1.0f);
	x = mat * x;
	x /= x.w;

	vec4 y(0.0f, 1.0f, 0.0f, 1.0f);
	y = mat * y;
	y /= y.w;

	vec4 z(0.0f, 0.0f, 1.0f, 1.0f);
	z = mat * z;
	z /= z.w;

	return S_OK;
}

API Camera::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
		pResMan->RemoveFromList(this);
	else if (refNum > 1)
		pResMan->DecrementRef(this);
	else
		LOG_WARNING("Camera::Free(): refNum == 0");

	delete this;

	return S_OK;
}

API Camera::GetType(RES_TYPE & type)
{
	type = RES_TYPE::CAMERA;
	return S_OK;
}
