#pragma once
#include "Common.h"
#include "GameObject.h"

class Camera : public GameObjectBase<ICamera>
{
	float _zNear = 0.10f;
	float _zFar = 1000.f;
	float _fovAngle = 60.0f;

	IInput *_pInput{nullptr};

	void _update();

public:

	Camera();
	virtual ~Camera();

	API GetViewProjectionMatrix(OUT mat4 *mat, float aspect) override;
	API GetFovAngle(OUT float *fovInDegrees) override;
	API Free() override;
};
