#pragma once
#include "Common.h"
#include "GameObject.h"

class Camera : public ICamera
{
	const float _zNear = 0.03f;
	const float _zFar = 200.f;
	const float _fovAngle = 60.0f;

public:

	Camera();

	API GetViewProjectionMatrix(mat4& mat, float aspect) override;
	API Free() override;
	API GetType(RES_TYPE& type) override;

	IGAMEOBJECT_IMPLEMENTATION
};
