#pragma once
#include "gameobject.h"

class Light : public GameObject
{
	float intensity_{1.0f};
	LIGHT_TYPE lightType_{ LIGHT_TYPE::DIRECT};

protected:
	virtual void Copy(GameObject *original) override;
	virtual void SaveYAML(void *yaml) override;
	virtual void LoadYAML(void *yaml) override;

public:

	Light();

	auto DLLEXPORT virtual GetIntensity() -> float { return intensity_; }
	auto DLLEXPORT virtual SetIntensity(float v) -> void { intensity_ = v; }
	auto DLLEXPORT virtual SetLightType(LIGHT_TYPE value) -> void { lightType_ = value; }
	auto DLLEXPORT virtual GetLightType() const -> LIGHT_TYPE { return lightType_; }

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
