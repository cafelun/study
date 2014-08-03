//=============================================================================
// Vertex.h by Frank Luna (C) 2005 All Rights Reserved.
//
// This file contains all of our vertex structures.  We will add to it
// as needed throughout the book.
//=============================================================================

#ifndef VERTEX_H
#define VERTEX_H

#include <d3dx9.h>


// Call in constructor and destructor, respectively, of derived application class.
void InitAllVertexDeclarations();
void DestroyAllVertexDeclarations();


//===============================================================
struct VertexPosColor
{
	VertexPosColor():pos(0.0f, 0.0f, 0.0f){}
    VertexPosColor(float x, float y, float z, D3DCOLOR c)
        : pos(x,y,z), color(c) {}
	VertexPosColor(const D3DXVECTOR3& v, D3DCOLOR c)
        : pos(v), color(c) {}

	D3DXVECTOR3 pos;
    D3DCOLOR color;
	static IDirect3DVertexDeclaration9* Decl;
};

#endif // VERTEX_H