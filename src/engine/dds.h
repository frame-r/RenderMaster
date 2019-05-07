#pragma once
#include "common.h"

class ICoreTexture;

ICoreTexture *createDDS(uint8_t *data, size_t size, TEXTURE_CREATE_FLAGS flags);
