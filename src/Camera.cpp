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

	float const tanHalfFovy = tan(_fovAngle / 2);
	P.el_2D[0][0] = 1 / (aspect * tanHalfFovy);
	P.el_2D[1][1] = 1 / (tanHalfFovy);
	P.el_2D[2][2] = (_zFar + _zNear) / (_zFar - _zNear);
	P.el_2D[2][3] = 1.0f;
	P.el_2D[3][2] = -(2 * _zFar * _zNear) / (_zFar - _zNear);
	P.el_2D[3][3] = 0.0f;

	V.el_2D[2][3] = -1.0f;

	mat = P * V;

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
