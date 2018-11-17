#include "Pch.h"
#include "Core.h"
#include "GameObject.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(GameObject, _pCore, RemoveRuntimeGameObject)
