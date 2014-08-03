/**----------------------------------------------------------------------------
 * \brief 행렬 사용
 * 파일: Matrices.cpp
 *
 * 설명 : 디바이스와 정점을 생성하는 법을 알았다.
 *        3차원 정점을 자유자재로 다루기 위해서는 4x4크기의 행렬을 사용해야 한다.
 *        기본적인 행렬변환에는 이동(translations), 회전(rotations), 크기(scaling)가 있다.
 *       the geometry with translations, rotations, scaling, and setting up our
 *       camera.
 *
 *       기하정보는 모델좌표계를 사용하는데, 이를 우선 3차원 월드좌표계로 변환해야 한다.
 *       이때 월드행렬이 사용된다.
 *       다시 월드좌표계의 기하정보를 카메라좌표계로 변환한다. 이때 사용되는것이 뷰행렬이다.
 *       다시 이 기하정보를 2차원상의 평면(viewport)에 투영해야 윈도우에 그려질수 있다.
 *       즉, world -> view -> proejction의 행렬변환을 거친뒤에 비로소 그려질수 있는 것이다.
 *       (물론, 이후 클리핑등의 처리가 추가적으로 이루어진다.)
 *
 *       OpenGL에서는 행렬연산 함수를 직접 작성해야 하겠지만, D3D에는 D3DX라는 유틸리티
 *       함수들이 다수 존재한다. 여기서는 D3DX계열 함수를 사용할 것이다.
 **-----------------------------------------------------------------------------
 */
#include <Windows.h>
#include <mmsystem.h>	/// TimeGetTime()함수를 사용하기 위해서 포함하는 헤더
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
*  전역변수
*------------------------------------------------------------------------------
*/
IDirect3D9*             g_pD3D = nullptr; /// D3D 디바이스를 생성할 D3D객체변수
IDirect3DDevice9*       g_pd3dDevice = nullptr; /// 렌더링에 사용될 D3D디바이스
D3DDEVTYPE              g_devType = D3DDEVTYPE_HAL;
DWORD                   g_requestedVP = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
D3DPRESENT_PARAMETERS   g_d3dPP;
IDirect3DVertexBuffer9* g_pVB = nullptr; /// 정점을 보관할 정점버퍼
IDirect3DIndexBuffer9*  g_pIV = nullptr;
ID3DXEffect*            g_pFx = nullptr;
D3DXHANDLE              g_hTech;
D3DXHANDLE              g_hWVP;
D3DXMATRIXA16           g_matWorld;
D3DXMATRIXA16           g_matView;
D3DXMATRIXA16           g_matProj;




/**-----------------------------------------------------------------------------
 * Direct3D 초기화
 *------------------------------------------------------------------------------
 */
HRESULT InitD3D( HWND hWnd )
{
    /// 디바이스를 생성하기위한 D3D객체 생성
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    /// 디바이스를 생성할 구조체
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    /// 디바이스 생성
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    /// 컬링기능을 끈다. 삼각형의 앞면, 뒷면을 모두 렌더링한다.
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    /// 정점에 색깔값이 있으므로, 광원기능을 끈다.
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
 * 기하정보 초기화
 *------------------------------------------------------------------------------
 */
HRESULT InitGeometry()
{
    /// 삼각형을 렌더링하기위해 세개의 정점을 선언
    VertexPosColor vertices[] =
    {
        VertexPosColor( -1.0f, 1.0f, 0.0f, 0xffff0000 ),
        VertexPosColor( 1.0f, 1.0f, 0.0f, 0xff00ff00 ),
        VertexPosColor(  1.0f, -1.0f, 0.0f, 0xff00ffff ),
    };

    /// 정점버퍼 생성
    /// 3개의 사용자정점을 보관할 메모리를 할당한다.
    /// FVF를 지정하여 보관할 데이터의 형식을 지정한다.
    if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(VertexPosColor)
        , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &g_pVB, NULL)))
    {
        return E_FAIL;
    }

    /// 정점버퍼를 값으로 채운다. 
    /// 정점버퍼의 Lock()함수를 호출하여 포인터를 얻어온다.
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
 * 초기화된 객체들 소거
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
 * 행렬 설정
 * 행렬은 세개가 있고, 각각 world, view, projection 행렬이다.
 *------------------------------------------------------------------------------
 */
VOID SetupMatrices()
{
	/// 월드행렬
    D3DXMATRIXA16 matWorld;

    
    UINT  iTime  = timeGetTime() % 1000;					/// float연산의 정밀도를 위해서 1000으로 나머지 연산한다.
    FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;		/// 1000밀리초마다 한바퀴씩(2 * pi) 회전 애니메이션 행렬을 만든다.
    D3DXMatrixRotationY( &matWorld, fAngle );				/// Y축을 회전행렬 생성
    //g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );	/// 생성한 회전행렬을 월드행렬로 디바이스에 설정
    g_matWorld = matWorld;

    /// 뷰행렬을 정의하기 위해서는 세가지값이 필요하다.    
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );							/// 1. 눈의 위치( 0, 3.0, -5)
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );						/// 2. 눈이 바라보는 위치( 0, 0, 0 )
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );							/// 3. 천정방향을 나타내는 상방벡터( 0, 1, 0 )
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );	/// 1,2,3의 값으로 뷰행렬 생성
    //g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );				/// 생성한 뷰행렬을 디바이스에 설정
    g_matView = matView;

    /// 프로젝션 행렬을 정의하기 위해서는 시야각(FOV=Field Of View)과 종횡비(aspect ratio), 클리핑 평면의 값이 필요하다.
    D3DXMATRIXA16 matProj;
    /// matProj   : 값이 설정될 행렬
    /// D3DX_PI/4 : FOV(D3DX_PI/4 = 45도)
    /// 1.0f      : 종횡비
    /// 1.0f      : 근접 클리핑 평면(near clipping plane)
    /// 100.0f    : 원거리 클리핑 평면(far clipping plane)
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );

    g_matProj = matProj;
    //g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_matProj);		/// 생성한 프로젝션 행렬을 디바이스에 설정
}



/**-----------------------------------------------------------------------------
 * 화면 그리기
 *------------------------------------------------------------------------------
 */
VOID Render()
{
    /// 후면버퍼를 파란색(0,0,255)으로 지운다.
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

    /// 렌더링 시작
    HR(g_pd3dDevice->BeginScene());
    {
        /// 월드,뷰,프로젝션 행렬을 설정한다.
        SetupMatrices();

        /// 정점버퍼의 삼각형을 그린다.
        /// 1. 정점정보가 담겨있는 정점버퍼를 출력 스트림으로 할당한다.
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

            /// 3. 기하 정보를 출력하기 위한 DrawPrimitive()함수 호출
            //HR(g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1));
            HR(g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1));

            HR(g_pFx->EndPass());
        }
        HR(g_pFx->End());

        /// 렌더링 종료
        HR(g_pd3dDevice->EndScene());
    }



    /// 후면버퍼를 보이는 화면으로!
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




/**-----------------------------------------------------------------------------
 * 윈도우 프로시져
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
 * 프로그램 시작점
 *------------------------------------------------------------------------------
 */
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    /// 윈도우 클래스 등록
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    /// 윈도우 생성
    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 03: Matrices",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );


    /// 윈도우 출력
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        if (SUCCEEDED(InitEffect()))
        {
            if (SUCCEEDED(InitGeometry()))
            {
                /// 메시지 루프
                MSG msg;
                ZeroMemory(&msg, sizeof(msg));
                while (msg.message != WM_QUIT)
                {
                    /// 메시지큐에 메시지가 있으면 메시지 처리
                    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                        /// 처리할 메시지가 없으면 Render()함수 호출
                        Render();
                }
            }
        }
    }

    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}



