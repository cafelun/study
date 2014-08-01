/**----------------------------------------------------------------------------
 * \brief 텍스쳐 매핑
 * 파일: Textures.cpp
 *
 * 설명: 3차원 오브젝트는 텍스쳐와 결합될때 더욱 현실감있게 된다. 텍스쳐란 마치
 *       벽지같은 것으로, 적당하게 늘이거나 줄여서 면에 붙이게 된다. 일반적으로
 *       텍스쳐는 이미지 파일(JPG,BMP,TGA등)을 D3DX계열의 함수를 사용해서 읽어들
 *       여서 사용하게 된다. 정점버퍼와 마찬가지로 Lock(), Unlock()함수를 사용해서
 *       메모리에 직접접근할 수도 있다. 이렇게 생성한 텍스쳐를 면에 붙이는 행위를
 *       텍스쳐 매핑이라고 한다.
 *
 *       텍스쳐는 기하정보와 연결되기위해서 텍스쳐좌표계를 갖게되는데, 각각의 정점
 *       은 이러한 텍스쳐 좌표를 포함하고 있어야한다. 일반적으로는 (u,v) 좌표계를
 *       사용하고 u,v의 값은 0.0 ~ 1.0 사이의 값이다. 텍스쳐 좌표는 최초에 설정될
 *       수도 있지만, 실시간으로 직접계산하여 더 다양한 효과를 낼수도 있다.
 *       (mirror, sphere매핑 등)
 *------------------------------------------------------------------------------
 */
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>

/// SHOW_HOW_TO_USE_TCI가 선언된것과 선언되지 않은것의 컴파일 결과를 반드시 비교해 보자.
/// #define SHOW_HOW_TO_USE_TCI

/**-----------------------------------------------------------------------------
 *  전역변수
 *------------------------------------------------------------------------------
 */
LPDIRECT3D9             g_pD3D       = NULL; /// D3D 디바이스를 생성할 D3D객체변수
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; /// 렌더링에 사용될 D3D디바이스
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; /// 정점을 보관할 정점버퍼
LPDIRECT3DTEXTURE9      g_pTexture   = NULL; /// 텍스쳐 정보

/// 사용자 정점을 정의할 구조체
/// 텍스쳐 좌표가 추가되었다는 것을 알수 있다.
struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; /// 3차원 좌표
    D3DCOLOR    color;    /// 색깔
#ifndef SHOW_HOW_TO_USE_TCI
    FLOAT       tu, tv;   /// 텍스쳐 좌표
#endif
};

/// 사용자 정점 구조체에 관한 정보를 나타내는 FVF값
#ifdef SHOW_HOW_TO_USE_TCI
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
#else
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif



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
    /// 복잡한 오브젝트를 그릴것이기때문에, 이번에는 Z버퍼가 필요하다.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    /// 디바이스 생성
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    /// 컬링기능을 끈다.
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    /// 조명기능을 끈다.
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    /// Z버퍼기능을 켠다.
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    return S_OK;
}




/**-----------------------------------------------------------------------------
 * 기하정보 초기화
 * 정점버퍼와 텍스쳐 생성
 *------------------------------------------------------------------------------
 */
HRESULT InitGeometry()
{
    /// D3DX계열 함수를 사용해서 파일로부터 텍스쳐 생성(banana.bmp)
    if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "banana.bmp", &g_pTexture ) ) )
    {
        /// 현재폴더에 파일이 없으면 상위폴더 검색
        if( FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, "..\\banana.bmp", &g_pTexture ) ) )
        {
        	/// 텍스쳐 생성 실패
            MessageBox(NULL, "Could not find banana.bmp", "Textures.exe", MB_OK);
            return E_FAIL;
        }
    }

    /// 정점버퍼 생성
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 50*2*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }

    /// 정점버퍼를 값으로 채운다.
    /// 텍스쳐의 u,v좌표값을 0.0 ~ 1.0 사이의 값으로 채워넣고 있다.
    CUSTOMVERTEX* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    for( DWORD i=0; i<50; i++ )
    {
        FLOAT theta = (2*D3DX_PI*i)/(50-1);

        pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta),-1.0f, cosf(theta) );
        pVertices[2*i+0].color    = 0xffffffff;
#ifndef SHOW_HOW_TO_USE_TCI								/// SHOW_HOW_TO_USE_TCI가 선언되어 있으면 텍스쳐 좌표를 생성하지 않는다.
        pVertices[2*i+0].tu       = ((FLOAT)i)/(50-1);	/// 텍스쳐의 u좌표 0/49, 1/49, 2/49 ... 49/49 (즉 0.0 ~ 1.0)
        pVertices[2*i+0].tv       = 1.0f;				/// 텍스쳐의 v좌표 1.0
#endif

        pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta), 1.0f, cosf(theta) );
        pVertices[2*i+1].color    = 0xff808080;
#ifndef SHOW_HOW_TO_USE_TCI								/// SHOW_HOW_TO_USE_TCI가 선언되어 있으면 텍스쳐 좌표를 생성하지 않는다.
        pVertices[2*i+1].tu       = ((FLOAT)i)/(50-1);	/// 텍스쳐의 u좌표 0/49, 1/49, 2/49 ... 49/49 (즉 0.0 ~ 1.0)
        pVertices[2*i+1].tv       = 0.0f;				/// 텍스쳐의 v좌표 0.0
#endif
    }
    g_pVB->Unlock();

    return S_OK;
}




/**-----------------------------------------------------------------------------
 * 초기화된 객체들 소거
 *------------------------------------------------------------------------------
 */
VOID Cleanup()
{
    if( g_pTexture != NULL )
        g_pTexture->Release();

    if( g_pVB != NULL )
        g_pVB->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}



/**-----------------------------------------------------------------------------
 * 행렬 설정
 *------------------------------------------------------------------------------
 */
VOID SetupMatrices()
{
	/// 월드행렬
    D3DXMATRIXA16 matWorld;
    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixRotationX( &matWorld, timeGetTime()/1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    /// 뷰행렬을 설정
    D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    /// 프로젝션 행렬 설정
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}




/**-----------------------------------------------------------------------------
 * 화면 그리기
 *------------------------------------------------------------------------------
 */
VOID Render()
{
    /// 후면버퍼와 Z버퍼를 지운다.
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

    /// 렌더링 시작
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        /// 월드,뷰,프로젝션 행렬을 설정한다.
        SetupMatrices();

        /// 생성한 텍스쳐를 0번 텍스쳐 스테이지에 올린다.
        /// 텍스쳐 스테이지는 여러장의 텍스쳐와 색깔정보를 섞어서 출력할때 사용된다.
        /// 여기서는 텍스쳐의 색깔과 정점의 색깔정보를 modulate연산으로 섞어서 출력한다.
        g_pd3dDevice->SetTexture( 0, g_pTexture );									/// 0번 텍스쳐 스테이지에 텍스쳐 고정
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );	/// MODULATE연산으로 색깔을 섞음
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );	/// 첫번째 섞을색은 텍스쳐 색
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );	/// 두번째 섞을색은 정점 색
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );	/// alpha연산은 사용하지 않음

    #ifdef SHOW_HOW_TO_USE_TCI
    	/// D3D의 텍스쳐 좌표 생성 기능을 사용하는 예를 보여준다.
    	/// 여기서는 카메라좌표계에서의 정점정보를 사용해서 텍스쳐 좌표를 생성한다.
    	/// 4x4크기의 텍스쳐 변환행렬을 텍스쳐좌표인덱스(TCI=Texture Coord Index)전달인자로 
    	/// 사용해서 x,y,z TCI좌표를 u,v텍스쳐 좌표로 변환한다.

    	/// 사용한 것은 단순히 (-1.0 ~ +1.0) 값을 (0.0 ~ 1.0) 사이의 값으로 변환하는 행렬이다.
    	/// world,view,projection변환을 거친 정점은 (-1.0 ~ +1.0)사이의 값을 갖게 된다.
        //  tu =  0.5*x + 0.5
        //  tv = -0.5*y + 0.5
        D3DXMATRIXA16 mat;
        mat._11 = 0.25f; mat._12 = 0.00f; mat._13 = 0.00f; mat._14 = 0.00f;
        mat._21 = 0.00f; mat._22 =-0.25f; mat._23 = 0.00f; mat._24 = 0.00f;
        mat._31 = 0.00f; mat._32 = 0.00f; mat._33 = 1.00f; mat._34 = 0.00f;
        mat._41 = 0.50f; mat._42 = 0.50f; mat._43 = 0.00f; mat._44 = 1.00f;

        g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat );												/// 텍스쳐 변환행렬
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );			/// 2차원 텍스쳐 사용
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );	/// 카메라 좌표계 변환
    #endif

        /// 정점버퍼의 내용을 그린다.
        g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
        g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2*50-2 );

        /// 렌더링 종료
        g_pd3dDevice->EndScene();
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
    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 05: Textures",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

    /// Direct3D 초기화
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        /// 기하정보 초기화
        if( SUCCEEDED( InitGeometry() ) )
        {
        	/// 윈도우 출력
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

        	/// 메시지 루프
            MSG msg;
            ZeroMemory( &msg, sizeof(msg) );
            while( msg.message!=WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                    Render();
            }
        }
    }

    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}



