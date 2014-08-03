//=============================================================================
// Vertex.cpp by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

#include "Vertex.h"
#include <dxerr.h>


#define SAFE_RELEASE(p) { if(p) p->Release(); p = nullptr; }


// extern variables
extern IDirect3DDevice9* g_pd3dDevice;


// Initialize static variables.
IDirect3DVertexDeclaration9* VertexPosColor::Decl = nullptr;


void InitAllVertexDeclarations()
{
	//===============================================================
	// VertexPos

	D3DVERTEXELEMENT9 VertexPosColorElements[] = 
	{
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        D3DDECL_END()
	};

    HRESULT hr = g_pd3dDevice->CreateVertexDeclaration(VertexPosColorElements, &VertexPosColor::Decl);
    if (FAILED(hr))
    {
        DXTrace(__FILE__, __LINE__, hr, "Fail InitAllVertexDeclarations", TRUE);
    }
}

void DestroyAllVertexDeclarations()
{
    SAFE_RELEASE(VertexPosColor::Decl);
}