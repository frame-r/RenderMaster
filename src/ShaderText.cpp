#pragma once
#include "Pch.h"
#include "ShaderText.h"
#include "Core.h"
#include "ResourceManager.h"

extern Core *_pCore;

SHARED_COM_CPP_IMPLEMENTATION(ShaderText, _pCore, RemoveSharedShaderText)

ShaderText::~ShaderText()
{
	delete vert;
	delete geom;
	delete frag;
}
