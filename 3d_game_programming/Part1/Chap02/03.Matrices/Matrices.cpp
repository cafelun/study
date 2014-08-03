/**----------------------------------------------------------------------------
 * \brief ��� ���
 * ����: Matrices.cpp
 *
 * ���� : ����̽��� ������ �����ϴ� ���� �˾Ҵ�.
 *        3���� ������ ��������� �ٷ�� ���ؼ��� 4x4ũ���� ����� ����ؾ� �Ѵ�.
 *        �⺻���� ��ĺ�ȯ���� �̵�(translations), ȸ��(rotations), ũ��(scaling)�� �ִ�.
 *       the geometry with translations, rotations, scaling, and setting up our
 *       camera.
 *
 *       ���������� ����ǥ�踦 ����ϴµ�, �̸� �켱 3���� ������ǥ��� ��ȯ�ؾ� �Ѵ�.
 *       �̶� ��������� ���ȴ�.
 *       �ٽ� ������ǥ���� ���������� ī�޶���ǥ��� ��ȯ�Ѵ�. �̶� ���Ǵ°��� ������̴�.
 *       �ٽ� �� ���������� 2�������� ���(viewport)�� �����ؾ� �����쿡 �׷����� �ִ�.
 *       ��, world -> view -> proejction�� ��ĺ�ȯ�� ��ģ�ڿ� ��μ� �׷����� �ִ� ���̴�.
 *       (����, ���� Ŭ���ε��� ó���� �߰������� �̷������.)
 *
 *       OpenGL������ ��Ŀ��� �Լ��� ���� �ۼ��ؾ� �ϰ�����, D3D���� D3DX��� ��ƿ��Ƽ
 *       �Լ����� �ټ� �����Ѵ�. ���⼭�� D3DX�迭 �Լ��� ����� ���̴�.
 **-----------------------------------------------------------------------------
 */
#include <Windows.h>
#include <mmsystem.h>	/// TimeGetTime()�Լ��� ����ϱ� ���ؼ� �����ϴ� ���
#include <d3dx9.h>
#include <dxerr.h>
#include "Vertex.h"




//===============================================================
// Clean up

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }


//===============================================================
// Debug

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
                            	{                                                  \
		HRESULT hr = x;                                \
		if(FAILED(hr))                                 \
                                {                                              \
			DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
                        		}                                              \
                            	}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif 




/**-----------------------------------------------------------------------------
*  ��������
*------------------------------------------------------------------------------
*/
IDirect3D9*             g_pD3D = nullptr; /// D3D ����̽��� ������ D3D��ü����
IDirect3DDevice9*       g_pd3dDevice = nullptr; /// �������� ���� D3D����̽�
D3DDEVTYPE              g_devType = D3DDEVTYPE_HAL;
DWORD                   g_requestedVP = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
D3DPRESENT_PARAMETERS   g_d3dPP;
IDirect3DVertexBuffer9* g_pVB = nullptr; /// ������ ������ ��������
IDirect3DIndexBuffer9*  g_pIV = nullptr;
ID3DXEffect*            g_pFx = nullptr;
D3DXHANDLE              g_hTech;
D3DXHANDLE              g_hWVP;
D3DXMATRIXA16           g_matWorld;
D3DXMATRIXA16           g_matView;
D3DXMATRIXA16           g_matProj;




/**-----------------------------------------------------------------------------
 * Direct3D �ʱ�ȭ
 *------------------------------------------------------------------------------
 */
HRESULT InitD3D( HWND hWnd )
{
    /// ����̽��� �����ϱ����� D3D��ü ����
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    /// ����̽��� ������ ����ü
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    /// ����̽� ����
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    /// �ø������ ����. �ﰢ���� �ո�, �޸��� ��� �������Ѵ�.
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    /// ������ ������ �����Ƿ�, ��������� ����.
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    return S_OK;
}


HRESULT InitEffect()
{
    ID3DXBuffer* errors = 0;
    HR(D3DXCreateEffectFromFile(g_pd3dDevice, "vertex.fx",
        0, 0, D3DXSHADER_DEBUG, 0, &g_pFx, &errors));

    if (errors)
        MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

    // Obtain handles.
    g_hTech = g_pFx->GetTechniqueByName("VertexTech");
    g_hWVP = g_pFx->GetParameterByName(0, "gWVP");

    return S_OK;
}




/**-----------------------------------------------------------------------------
 * �������� �ʱ�ȭ
 *------------------------------------------------------------------------------
 */
HRESULT InitGeometry()
{
    /// �ﰢ���� �������ϱ����� ������ ������ ����
    VertexPosColor vertices[] =
    {
        VertexPosColor( -1.0f, 1.0f, 0.0f, 0xffff0000 ),
        VertexPosColor( 1.0f, 1.0f, 0.0f, 0xff00ff00 ),
        VertexPosColor(  1.0f, -1.0f, 0.0f, 0xff00ffff ),
    };

    /// �������� ����
    /// 3���� ����������� ������ �޸𸮸� �Ҵ��Ѵ�.
    /// FVF�� �����Ͽ� ������ �������� ������ �����Ѵ�.
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(VertexPosColor)
        , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &g_pVB, NULL)))
    {
        return E_FAIL;
    }

    /// �������۸� ������ ä���. 
    /// ���������� Lock()�Լ��� ȣ���Ͽ� �����͸� ���´�.
    VertexPosColor* pVertices = nullptr;
    if( FAILED( g_pVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    CopyMemory( pVertices, vertices, sizeof(vertices) );
    g_pVB->Unlock();



    // Obtain a pointer to a new index buffer.
    HR(g_pd3dDevice->CreateIndexBuffer(3 * sizeof(WORD), D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIV, 0));

    // Now lock it to obtain a pointer to its internal data, and write the

    WORD* k = 0;

    HR(g_pIV->Lock(0, 0, (void**)&k, 0));

    // Front face.
    k[0] = 0; k[1] = 1; k[2] = 2;

    HR(g_pIV->Unlock());






    InitAllVertexDeclarations();

    return S_OK;

}




/**-----------------------------------------------------------------------------
 * �ʱ�ȭ�� ��ü�� �Ұ�
 *------------------------------------------------------------------------------
 */
VOID Cleanup()
{
    if (g_pVB != nullptr)
        g_pVB->Release();

    if (g_pIV != nullptr)
        g_pIV->Release();

    if (g_pFx != nullptr)
        g_pFx->Release();

    DestroyAllVertexDeclarations();



    if (g_pd3dDevice != nullptr)
        g_pd3dDevice->Release();

    if (g_pD3D != nullptr)
        g_pD3D->Release();
}



/**-----------------------------------------------------------------------------
 * ��� ����
 * ����� ������ �ְ�, ���� world, view, projection ����̴�.
 *------------------------------------------------------------------------------
 */
VOID SetupMatrices()
{
	/// �������
    D3DXMATRIXA16 matWorld;

    
    UINT  iTime  = timeGetTime() % 1000;					/// float������ ���е��� ���ؼ� 1000���� ������ �����Ѵ�.
    FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;		/// 1000�и��ʸ��� �ѹ�����(2 * pi) ȸ�� �ִϸ��̼� ����� �����.
    D3DXMatrixRotationY( &matWorld, fAngle );				/// Y���� ȸ����� ����
    //g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );	/// ������ ȸ������� ������ķ� ����̽��� ����
    g_matWorld = matWorld;

    /// ������� �����ϱ� ���ؼ��� ���������� �ʿ��ϴ�.    
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );							/// 1. ���� ��ġ( 0, 3.0, -5)
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );						/// 2. ���� �ٶ󺸴� ��ġ( 0, 0, 0 )
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );							/// 3. õ�������� ��Ÿ���� ��溤��( 0, 1, 0 )
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );	/// 1,2,3�� ������ ����� ����
    //g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );				/// ������ ������� ����̽��� ����
    g_matView = matView;

    /// �������� ����� �����ϱ� ���ؼ��� �þ߰�(FOV=Field Of View)�� ��Ⱦ��(aspect ratio), Ŭ���� ����� ���� �ʿ��ϴ�.
    D3DXMATRIXA16 matProj;
    /// matProj   : ���� ������ ���
    /// D3DX_PI/4 : FOV(D3DX_PI/4 = 45��)
    /// 1.0f      : ��Ⱦ��
    /// 1.0f      : ���� Ŭ���� ���(near clipping plane)
    /// 100.0f    : ���Ÿ� Ŭ���� ���(far clipping plane)
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );

    g_matProj = matProj;
    //g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_matProj);		/// ������ �������� ����� ����̽��� ����
}



/**-----------------------------------------------------------------------------
 * ȭ�� �׸���
 *------------------------------------------------------------------------------
 */
VOID Render()
{
    /// �ĸ���۸� �Ķ���(0,0,255)���� �����.
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

    /// ������ ����
    HR(g_pd3dDevice->BeginScene());
    {
        /// ����,��,�������� ����� �����Ѵ�.
        SetupMatrices();

        /// ���������� �ﰢ���� �׸���.
        /// 1. ���������� ����ִ� �������۸� ��� ��Ʈ������ �Ҵ��Ѵ�.
        HR(g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(VertexPosColor)));

        HR(g_pd3dDevice->SetIndices(g_pIV));

        HR(g_pd3dDevice->SetVertexDeclaration(VertexPosColor::Decl));

        HR(g_pFx->SetTechnique(g_hTech));

        HR(g_pFx->SetMatrix(g_hWVP, &(g_matWorld * g_matView * g_matProj)));

        // Begin passes.
        UINT numPasses = 0;

        HR(g_pFx->Begin(&numPasses, 0));

        for (UINT i = 0; i < numPasses; ++i)
        {
            HR(g_pFx->BeginPass(i));

            /// 3. ���� ������ ����ϱ� ���� DrawPrimitive()�Լ� ȣ��
            //HR(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1));
            HR(g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1));

            HR(g_pFx->EndPass());
        }
        HR(g_pFx->End());

        /// ������ ����
        HR(g_pd3dDevice->EndScene());
    }



    /// �ĸ���۸� ���̴� ȭ������!
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




/**-----------------------------------------------------------------------------
 * ������ ���ν���
 *------------------------------------------------------------------------------
 */
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}




/**-----------------------------------------------------------------------------
 * ���α׷� ������
 *------------------------------------------------------------------------------
 */
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    /// ������ Ŭ���� ���
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    /// ������ ����
    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 03: Matrices",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );


    /// ������ ���
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        if (SUCCEEDED(InitEffect()))
        {
            if (SUCCEEDED(InitGeometry()))
            {
                /// �޽��� ����
                MSG msg;
                ZeroMemory(&msg, sizeof(msg));
                while (msg.message != WM_QUIT)
                {
                    /// �޽���ť�� �޽����� ������ �޽��� ó��
                    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                        /// ó���� �޽����� ������ Render()�Լ� ȣ��
                        Render();
                }
            }
        }
    }

    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}



