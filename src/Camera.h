#pragma once
#include "Common.h"
#include "GameObject.h"

class Camera : public GameObjectBase<ICamera>
{
	const float _zNear = 0.10f;
	const float _zFar = 1000.f;
	const float _fovAngle = 60.0f;

	IInput *_pInput{nullptr};

	void _update();

public:

	Camera();

	API GetViewProjectionMatrix(OUT mat4 *mat, float aspect) override;
	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};
