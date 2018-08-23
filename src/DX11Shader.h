#pragma once
#include "Common.h"
#include <d3d11_2.h>


class DX11Shader : public ICoreShader
{
	ID3D11VertexShader *pVertex{nullptr};
	ID3D11GeometryShader *pGeometry{nullptr};
	ID3D11PixelShader *pFragment{nullptr};

public:

	ID3D11VertexShader* vs() const { return pVertex; }
	ID3D11GeometryShader* gs() const { return pGeometry; }
	ID3D11PixelShader* fs() const { return pFragment; }


	DX11Shader(ID3D11VertexShader* pVeretxIn, ID3D11GeometryShader* pGeometryIn, ID3D11PixelShader* pFragmentIn) : 
		pVertex(pVeretxIn), pGeometry(pGeometryIn), pFragment(pFragmentIn){}
	virtual ~DX11Shader(){}
	
	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};

