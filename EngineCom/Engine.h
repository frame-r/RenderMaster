#pragma once
#define INITGUID
#include <Unknwn.h>

#define API HRESULT

// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
DEFINE_GUID(IID_Core,
	0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

class ICore : public IUnknown
{
public:

	virtual API StartEngine() = 0;
	virtual API CloseEngine() = 0;
};