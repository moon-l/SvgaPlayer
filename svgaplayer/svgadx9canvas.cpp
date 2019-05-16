#include <d3d9.h>
#include <d3dx9.h>
#include "svgadx9canvas.h"

#define WIN_CLASS L"SvgaDx9CanvasClass"
#define WIN_NAME L"SvgaDx9Canvas"

class SvgaDx9CanvasPrivate
{
public:
	SvgaDx9CanvasPrivate(SvgaDx9Canvas* q);
	~SvgaDx9CanvasPrivate();

	bool init();
	void release();

	bool setup(int width, int height);
	void reset();

	void begin();
	void end();

	void draw(const QString& key, QPixmap& pix, QRect& layout, QTransform& transform, float alpha);

private:
	SvgaDx9Canvas* q_ptr;
	Q_DECLARE_PUBLIC(SvgaDx9Canvas)

private:
	HMODULE					m_hD3d9Module;
	HMODULE					m_hD3d9XModule;
	LPDIRECT3D9				m_pD3D9;

	HWND					m_hwnd;
	LPDIRECT3DDEVICE9		m_pD3DDevice;
	LPD3DXSPRITE			m_pSprite;
	QMap<QString, IDirect3DTexture9*>	m_textures;

	int						m_videoWidth;
	int						m_videoHeight;
};

SvgaDx9CanvasPrivate::SvgaDx9CanvasPrivate(SvgaDx9Canvas* q)
: q_ptr(q)
, m_hD3d9Module(NULL)
, m_hD3d9XModule(NULL)
, m_pD3D9(NULL)
, m_hwnd(NULL)
, m_pD3DDevice(NULL)
, m_pSprite(NULL)
, m_videoWidth(0)
, m_videoHeight(0)
{

}

SvgaDx9CanvasPrivate::~SvgaDx9CanvasPrivate()
{
	
}

bool SvgaDx9CanvasPrivate::init()
{
	m_hD3d9Module = LoadLibrary(L"d3d9.dll");
	if (m_hD3d9Module == NULL)
	{
		return false;
	}

	m_hD3d9XModule = LoadLibrary(L"d3dx9_30.dll");
	if (m_hD3d9XModule == NULL)
	{
		release();
		return false;
	}

	typedef IDirect3D9* (WINAPI *LPDIRECT3DCREATE9)(UINT);
	LPDIRECT3DCREATE9 Direct3DCreate9Ptr = (LPDIRECT3DCREATE9)GetProcAddress(m_hD3d9Module, "Direct3DCreate9");
	if (Direct3DCreate9Ptr) 
	{ 
		m_pD3D9 = Direct3DCreate9Ptr(D3D_SDK_VERSION);
	}

	if (!m_pD3D9)
	{
		release();
		return false;
	}

	return true;
}

void SvgaDx9CanvasPrivate::release()
{
	reset();

	if (m_pD3D9)
	{
		m_pD3D9->Release();
		m_pD3D9 = NULL;
	}

	if (m_hD3d9Module)
	{
		FreeLibrary(m_hD3d9Module);
		m_hD3d9Module = NULL;
	}

	if (m_hD3d9XModule)
	{
		FreeLibrary(m_hD3d9XModule);
		m_hD3d9XModule = NULL;
	}

	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

bool SvgaDx9CanvasPrivate::setup(int width, int height)
{
	if (!m_pD3D9)
	{
		return false;
	}

	if (width == m_videoWidth || height == m_videoHeight)
	{
		return true;
	}

	reset();

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	MoveWindow(m_hwnd, rc.left, rc.top, width, height, FALSE);

	// 全屏与非全屏状态下的参数设置
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth  = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount  = 1;
	d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed         = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	// 检测顶点运算
	D3DCAPS9 caps;
	m_pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	DWORD flags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_NOWINDOWCHANGES;
	}
	else
	{
		flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES;
	}

	// 创建3D设备 
	HRESULT hr = m_pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, flags, &d3dpp, &m_pD3DDevice);
	if (hr != D3D_OK)
	{
		return false;
	}

	typedef HRESULT (WINAPI *LPD3DXCreateSprite)(LPDIRECT3DDEVICE9, LPD3DXSPRITE*);
	LPD3DXCreateSprite D3DXCreateSpritePtr = (LPD3DXCreateSprite)GetProcAddress(m_hD3d9XModule, "D3DXCreateSprite"); 
	if (D3DXCreateSpritePtr) 
	{ 
		D3DXCreateSpritePtr(m_pD3DDevice, &m_pSprite);
	}

	if (!m_pSprite)
	{
		reset();
		return false;
	}

	m_videoWidth = width;
	m_videoHeight = height;

	return true;
}

void SvgaDx9CanvasPrivate::reset()
{
	for (QMap<QString, IDirect3DTexture9*>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		iter.value()->Release();
	}
	m_textures.clear();

	if (m_pSprite)
	{
		m_pSprite->Release();
		m_pSprite = NULL;
	}

	if (m_pD3DDevice)
	{
		m_pD3DDevice->Release();
		m_pD3DDevice = NULL;
	}
}

void SvgaDx9CanvasPrivate::begin()
{
	if (!m_pD3DDevice || !m_pSprite)
	{
		return;
	}

	m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0,0,0,0), 1.0f, 0);
	m_pD3DDevice->BeginScene();

	m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_pD3DDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);
	m_pD3DDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
}

void SvgaDx9CanvasPrivate::end()
{
	if (m_pD3DDevice && m_pSprite)
	{
		m_pSprite->End();
		m_pD3DDevice->EndScene();
		m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
	}
}

void SvgaDx9CanvasPrivate::draw(const QString& key, QPixmap& pix, QRect& layout, QTransform& transform, float alpha)
{
	if (!m_pD3DDevice || !m_pSprite)
	{
		return;
	}

	IDirect3DTexture9* texture = m_textures.value(key, NULL);
	if (!texture)
	{
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QIODevice::ReadWrite);
		pix.save(&buffer, "png", 100);

		typedef HRESULT (WINAPI *LPD3DXCreateTextureFromFileInMemoryEx)(LPDIRECT3DDEVICE9, LPCVOID, UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*, LPDIRECT3DTEXTURE9*);
		LPD3DXCreateTextureFromFileInMemoryEx D3DXCreateTextureFromFileInMemoryExPtr = (LPD3DXCreateTextureFromFileInMemoryEx)GetProcAddress(m_hD3d9XModule, "D3DXCreateTextureFromFileInMemoryEx");
		if (D3DXCreateTextureFromFileInMemoryExPtr)
		{
			D3DXCreateTextureFromFileInMemoryExPtr(m_pD3DDevice,(void*)buffer.data().data(), buffer.size(), pix.width(), pix.height(), D3DFMT_A8R8G8B8, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &texture);
			if (texture)
			{
				m_textures[key] = texture;
			}
		}

		//typedef HRESULT (WINAPI *LPD3DXSaveTextureToFileW)(LPCWSTR, D3DXIMAGE_FILEFORMAT, LPDIRECT3DBASETEXTURE9, CONST PALETTEENTRY*);
		//LPD3DXSaveTextureToFileW D3DXSaveTextureToFileWPtr = (LPD3DXSaveTextureToFileW)GetProcAddress(m_hD3d9XModule, "D3DXSaveTextureToFileW");
		//if (D3DXSaveTextureToFileWPtr)
		//{
		//	D3DXSaveTextureToFileWPtr(L"test.jpg", D3DXIFF_JPG, texture, NULL);
		//}
	}

	if (!texture)
	{
		return;
	}

	RECT rc;
	rc.left = layout.x();
	rc.top = layout.y();
	rc.right = layout.right();
	rc.bottom = layout.bottom();

	D3DXMATRIX mat(
		transform.m11(), transform.m12(), 0, 0,
		transform.m21(), transform.m22(), 0, 0,
		0, 0, 1, 0,
		transform.dx(), transform.dy(), 0, 1
		); 
	m_pSprite->SetTransform(&mat); 

	D3DXCOLOR color(255.0f, 255.0f, 255.0f, alpha);
	m_pSprite->Draw(texture, &rc, NULL, NULL, color);
}

SvgaDx9Canvas::SvgaDx9Canvas()
: m_ptr(new SvgaDx9CanvasPrivate(this))
{
	m_ptr->init();
}

SvgaDx9Canvas::~SvgaDx9Canvas()
{
	if (m_ptr)
	{
		m_ptr->release();
		delete m_ptr;
		m_ptr = NULL;
	}
}

void SvgaDx9Canvas::init(HWND parent)
{
	Q_D(SvgaDx9Canvas);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= DefWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WIN_CLASS;
	wcex.hIconSm		= NULL;
	RegisterClassEx(&wcex);

	d->m_hwnd = CreateWindow(WIN_CLASS, WIN_NAME, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, GetModuleHandle(NULL), NULL);
}

void SvgaDx9Canvas::setPos(int x, int y)
{
	Q_D(SvgaDx9Canvas);
	MoveWindow(d->m_hwnd, x, y, d->m_videoWidth, d->m_videoHeight, FALSE);
}

void SvgaDx9Canvas::setVisible(bool visible)
{
	Q_D(SvgaDx9Canvas);
	ShowWindow(d->m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

void SvgaDx9Canvas::setVideoSize(int width, int height)
{
	Q_D(SvgaDx9Canvas);
	d->setup(width, height);
}

void SvgaDx9Canvas::begin()
{
	Q_D(SvgaDx9Canvas);
	d->begin();
}

void SvgaDx9Canvas::end()
{
	Q_D(SvgaDx9Canvas);
	d->end();
}

void SvgaDx9Canvas::draw(const QString& key, QPixmap& pix, QRect& layout, QTransform& transform, float alpha)
{
	Q_D(SvgaDx9Canvas);
	d->draw(key, pix, layout, transform, alpha);
}
