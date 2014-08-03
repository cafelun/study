/**-----------------------------------------------------------------------------
 * \brief �������� ����
 * ����: Vertices.cpp
 *
 * ����: �̹����� ������ �������ϴ� ���� �����ϰ� �ȴ�. ���⼭ �츮�� ��������
 *       (Vertex Buffer)������ ���Եɰ��̴�. �������۶� ������ �����ϱ� ���� 
 *       D3D�� ��ü�ε�, ����ڰ� FVF(Flexible Vertex Format)�� ����Ͽ� ������
 *       �� ������ ������ �� �ִ�. �̹� �н������� ��ȯ�� ����ó���� �Ϸ�� ����
 *       �� �̿��� ���̴�.
 *------------------------------------------------------------------------------
 */
#include <d3d9.h>
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
D3DXHANDLE              ghTech;


/**-----------------------------------------------------------------------------
 * Direct3D �ʱ�ȭ
 *------------------------------------------------------------------------------
 */
HRESULT InitD3D( HWND hWnd )
{
    /// ����̽��� �����ϱ����� D3D��ü ����
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return E_FAIL;

    /// ����̽��� ������ ����ü
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = 0;
    d3dpp.BackBufferHeight = 0;
    d3dpp.EnableAutoDepthStencil = true;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

    /// ����̽� ����
    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &g_pd3dDevice)))
    {
        return E_FAIL;
    }

    /// ����̽� ���������� ó���Ұ�� ���⿡�� �Ѵ�.

    return S_OK;

    //ZeroMemory(&g_d3dPP, sizeof(g_d3dPP));

    //// Step 1: Create the IDirect3D9 object.

    //g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    //if (!g_pD3D)
    //{
    //    MessageBox(0, "Direct3DCreate9 FAILED", 0, 0);
    //    PostQuitMessage(0);
    //}


    //// Step 2: Verify hardware support for specified formats in windowed and full screen modes.

    //D3DDISPLAYMODE mode;
    //g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
    //HR(g_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, g_devType, mode.Format, mode.Format, true));
    //HR(g_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, g_devType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false));

    //// Step 3: Check for requested vertex processing and pure device.

    //D3DCAPS9 caps;
    //HR(g_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, g_devType, &caps));

    //DWORD devBehaviorFlags = 0;
    //if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    //    devBehaviorFlags |= g_requestedVP;
    //else
    //    devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    //// If pure device and HW T&L supported
    //if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE &&
    //    devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
    //    devBehaviorFlags |= D3DCREATE_PUREDEVICE;

    //// Step 4: Fill out the D3DPRESENT_PARAMETERS structure.

    //g_d3dPP.BackBufferWidth = 0;
    //g_d3dPP.BackBufferHeight = 0;
    //g_d3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
    //g_d3dPP.BackBufferCount = 1;
    //g_d3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
    //g_d3dPP.MultiSampleQuality = 0;
    //g_d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
    //g_d3dPP.hDeviceWindow = hWnd;
    //g_d3dPP.Windowed = true;
    //g_d3dPP.EnableAutoDepthStencil = true;
    //g_d3dPP.AutoDepthStencilFormat = D3DFMT_D24S8;
    //g_d3dPP.Flags = 0;
    //g_d3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    //g_d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


    //// Step 5: Create the device.

    //HR(g_pD3D->CreateDevice(
    //    D3DADAPTER_DEFAULT, // primary adapter
    //    g_devType,           // device type
    //    hWnd,          // window associated with device
    //    devBehaviorFlags,   // vertex processing
    //    &g_d3dPP,            // present parameters
    //    &g_pd3dDevice));      // return created device

    //return S_OK;
}



HRESULT InitEffect()
{
    ID3DXBuffer* errors = 0;
    HR(D3DXCreateEffectFromFile(g_pd3dDevice, "vertex.fx",
        0, 0, D3DXSHADER_DEBUG, 0, &g_pFx, &errors));

    if (errors)
        MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);

    // Obtain handles.
    ghTech = g_pFx->GetTechniqueByName("VertexTech");

    return S_OK;
}




/**-----------------------------------------------------------------------------
 * �������۸� �����ϰ� �������� ä���ִ´�.
 * �������۶� �⺻������ ���������� �����ִ� �޸𸮺��̴�.
 * �������۸� ������ �������� �ݵ�� Lock()�� Unlock()���� �����͸� ���� 
 * ���������� �������ۿ� ��־�� �Ѵ�.
 * ���� D3D�� �ε������۵� ��밡���ϴٴ� ���� �������.
 * �������۳� �ε������۴� �⺻ �ý��� �޸𸮿ܿ� ����̽� �޸�(����ī�� �޸�)
 * �� �����ɼ� �ִµ�, ��κ��� ����ī�忡���� �̷��� �Ұ�� ��û�� �ӵ��� �����
 * ���� �� �ִ�.
 *------------------------------------------------------------------------------
 */
HRESULT InitVB()
{
    /// �ﰢ���� �������ϱ����� ������ ������ ����
    VertexPosColor vertices[] =
    {
        VertexPosColor( -1.0f, 1.0f, 0.5f, 0xffff0000 ),
        VertexPosColor( 1.0f, 1.0f, 0.5f, 0xff00ff00 ),
        VertexPosColor(  1.0f, -1.0f, 0.5f, 0xff00ffff ),
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
 * �ʱ�ȭ ��ü�� �Ұ�
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
 * ȭ�� �׸���
 *------------------------------------------------------------------------------
 */
VOID Render()
{
    /// �ĸ���۸� �Ķ���(0,0,255)���� �����.
    HR(g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0));
    //HR(g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0));

    /// ������ ����
    HR(g_pd3dDevice->BeginScene());
    {
        /// ���������� �ﰢ���� �׸���.
        /// 1. ���������� ����ִ� �������۸� ��� ��Ʈ������ �Ҵ��Ѵ�.
        HR(g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(VertexPosColor)));

        HR(g_pd3dDevice->SetIndices(g_pIV));

        HR(g_pd3dDevice->SetVertexDeclaration(VertexPosColor::Decl))

        HR(g_pFx->SetTechnique(ghTech))

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
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    /// ������ Ŭ���� ���
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "D3D Tutorial", NULL };
    RegisterClassEx( &wc );

    /// ������ ����
    HWND hWnd = CreateWindow( "D3D Tutorial", "D3D Tutorial 02: Vertices",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );


    /// ������ ���
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);


    if (SUCCEEDED(InitD3D(hWnd)))
    {
        if (SUCCEEDED(InitEffect()))
        {
            if (SUCCEEDED(InitVB()))
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

	/// ��ϵ� Ŭ���� �Ұ�
    UnregisterClass( "D3D Tutorial", wc.hInstance );
    return 0;
}
