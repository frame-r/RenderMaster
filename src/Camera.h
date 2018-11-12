#pragma once
#include "Common.h"
#include "GameObject.h"
#include "Serialization.h"

class Camera : public GameObjectBase<ICamera>
{
	float _zNear = 0.10f;
	float _zFar = 1000.f;
	float _fovAngle = 60.0f;

	IInput *_pInput{nullptr};

	void _update();

	//friend YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g);
	//friend void loadResource(YAML::Node& n, IGameObject *go);

public:

	Camera();
	virtual ~Camera();

	API GetViewProjectionMatrix(OUT mat4 *mat, float aspect) override;
	API GetFovAngle(OUT float *fovInDegrees) override;
	API Copy(OUT ICamera *copy) override;

	RUNTIME_COM_HEADER_IMPLEMENTATION
};
