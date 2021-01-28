#include <d3d11.h>
#include <d3dx11tex.h>
#include <dxgi.h>
#include "svgadx11canvas.h"
#include "svgapath.h"

#define WIN_DX11_CLASS L"SvgaDx11CanvasClass"
#define WIN_DX11_NAME L"SvgaDx11Canvas"

#define DX_SHADER_CODE(x) #x

const char* hlsl = DX_SHADER_CODE(
cbuffer cbConst : register(b0)\n
{\n
    matrix view;\n
};\n

cbuffer cbSprite : register(b1)\n
{\n
	matrix model;\n
	float alpha;\n
	float3 dummy;\n
};\n

Texture2D tex : register(t0);\n
SamplerState sam\n
{\n
    Filter = MIN_MAG_MIP_LINEAR;\n
    AddressU = Wrap;\n
    AddressV = Wrap;\n
};\n

struct VS_INPUT\n
{\n
    float4 pos : POSITION;\n
    float2 tex : TEXCOORD0;\n
};\n

struct PS_INPUT\n
{\n
	float4 pos : SV_POSITION;\n
	float2 tex : TEXCOORD0;\n
};\n

PS_INPUT VS(VS_INPUT input)\n
{\n
    PS_INPUT output = (PS_INPUT)0;\n
    output.pos = mul(input.pos, model);\n
	output.pos = mul(output.pos, view);\n
	//output.pos =  float4(output.pos.xy, 0, 1);
	output.tex = input.tex;\n
	return output;\n
}\n

float4 PS(PS_INPUT input) : SV_Target\n
{\n
    float4 color = tex.Sample(sam, input.tex);\n
	if(color.a < 0.1)\n
		discard;\n
	return float4(color.bgr, color.a * alpha);
}
);

struct ShaderVertex
{
	FLOAT pos[3];
	FLOAT tex[2];
};

struct ShaderConst
{
	FLOAT view[16];
};

struct ShaderSprite
{
	FLOAT model[16];
	FLOAT alpha;
	FLOAT dummy[3];
};

struct Dx11TextureInfo
{
	QString				clipPath;
	bool				dynamic;
	ID3D11Texture2D*	texture;
	ID3D11ShaderResourceView* view;

	Dx11TextureInfo() : dynamic(false), texture(NULL), view(NULL)
	{

	}
};

class SvgaDx11CanvasPrivate
{
public:
	SvgaDx11CanvasPrivate(SvgaDx11Canvas* q);
	~SvgaDx11CanvasPrivate();

	bool init();
	void release();

	bool setup(int width, int height);
	void reset();

	bool begin();
	void end();

	void draw(DrawItem* item);

private:
	ID3D11Texture2D* _loadImage(QImage& img, ID3D11ShaderResourceView** view);
	bool _updateImage(QImage& img, ID3D11Texture2D* texture);
	ID3D11ShaderResourceView* _getTexture(const QString& key, QPixmap& pix, const QString& clipPath, bool dynamic);
	bool _compileShader(LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

private:
	SvgaDx11Canvas* q_ptr;
	Q_DECLARE_PUBLIC(SvgaDx11Canvas)

private:
	HMODULE					m_hD3d11Module;
	HMODULE					m_hD3d11XModule;
	HMODULE					m_hDXGIModule;

	HWND					m_hwnd;
	ID3D11Device*			m_pDevice;
	ID3D11DeviceContext*	m_pContext;
	IDXGISwapChain*			m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;

	ID3D11VertexShader*		m_pVertexShader;
	ID3D11PixelShader*		m_pPixelShader;
	ID3D11InputLayout*		m_pVertexLayout;
	ID3D11Buffer*			m_pVertexBuffer;
	ID3D11Buffer*			m_pIndexBuffer;
	ID3D11Buffer*           m_pCBConst;
	ID3D11Buffer*           m_pCBSprite;

	QMap<QString, Dx11TextureInfo>	m_textures;

	int						m_videoWidth;
	int						m_videoHeight;
};

SvgaDx11CanvasPrivate::SvgaDx11CanvasPrivate(SvgaDx11Canvas* q)
: q_ptr(q)
, m_hD3d11Module(NULL)
, m_hD3d11XModule(NULL)
, m_hDXGIModule(NULL)
, m_hwnd(NULL)
, m_pDevice(NULL)
, m_pContext(NULL)
, m_pSwapChain(NULL)
, m_pRenderTargetView(NULL)
, m_pVertexShader(NULL)
, m_pPixelShader(NULL)
, m_pVertexLayout(NULL)
, m_pVertexBuffer(NULL)
, m_pIndexBuffer(NULL)
, m_pCBConst(NULL)
, m_pCBSprite(NULL)
, m_videoWidth(0)
, m_videoHeight(0)
{

}

SvgaDx11CanvasPrivate::~SvgaDx11CanvasPrivate()
{
	
}

bool SvgaDx11CanvasPrivate::init()
{
	m_hD3d11Module = LoadLibrary(L"d3d11.dll");
	if (m_hD3d11Module == NULL)
	{
		return false;
	}

	m_hD3d11XModule = LoadLibrary(L"d3dx11_43.dll");
	if (m_hD3d11XModule == NULL)
	{
		release();
		return false;
	}

	m_hDXGIModule = LoadLibrary(L"dxgi.dll");
	if (m_hDXGIModule == NULL)
	{
		release();
		return false;
	}


	PFN_D3D11_CREATE_DEVICE pfnD3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(m_hD3d11Module, "D3D11CreateDevice");
	if (!pfnD3D11CreateDevice)
	{
		release();
		return false;
	}

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr = S_OK;
	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		D3D_FEATURE_LEVEL level;
		hr = pfnD3D11CreateDevice(NULL, driverTypes[driverTypeIndex], NULL, 0, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_pDevice, &level, &m_pContext);
		if(SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
	{
		release();
		return false;
	}

	return true;
}

void SvgaDx11CanvasPrivate::release()
{
	reset();

	if (m_pContext)
	{
		m_pContext->Release();
		m_pContext = NULL;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}

	if (m_hDXGIModule)
	{
		FreeLibrary(m_hDXGIModule);
		m_hDXGIModule = NULL;
	}

	if (m_hD3d11XModule)
	{
		FreeLibrary(m_hD3d11XModule);
		m_hD3d11XModule = NULL;
	}

	if (m_hD3d11Module)
	{
		FreeLibrary(m_hD3d11Module);
		m_hD3d11Module = NULL;
	}

	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

bool SvgaDx11CanvasPrivate::setup(int width, int height)
{
	if (!m_pDevice)
	{
		return false;
	}

	if (width == m_videoWidth && height == m_videoHeight)
	{
		return true;
	}

	reset();

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	MoveWindow(m_hwnd, rc.left, rc.top, width, height, FALSE);


	HRESULT hr = S_OK;

	IDXGIFactory1* dxgiFactory = NULL;
	{
		IDXGIDevice* dxgiDevice = NULL;
		hr = m_pDevice->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = NULL;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
	{
		reset();
		return false;
	}

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = dxgiFactory->CreateSwapChain(m_pDevice, &sd, &m_pSwapChain);

	dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();
	if (FAILED(hr))
	{
		reset();
		return false;
	}

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{
		reset();
		return false;
	}

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		reset();
		return false;
	}

	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pContext->RSSetViewports(1, &vp);

	m_videoWidth = width;
	m_videoHeight = height;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	if(!_compileShader("VS", "vs_4_0", &pVSBlob))
	{
		reset();
		return false;
	}

	// Create the vertex shader
	hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if(FAILED(hr))
	{
		pVSBlob->Release();
		reset();
		return false;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	// Set the input layout
	m_pContext->IASetInputLayout(m_pVertexLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	if(!_compileShader("PS", "ps_4_0", &pPSBlob))
	{
		reset();
		return false;
	}

	// Create the pixel shader
	hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	D3D11_SUBRESOURCE_DATA InitData = {};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ShaderVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	// Set vertex buffer
	UINT stride = sizeof(ShaderVertex);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	WORD indices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_pDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	// Set index buffer
	m_pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShaderConst);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pDevice->CreateBuffer(&bd, NULL, &m_pCBConst);
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ShaderSprite);
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bd, NULL, &m_pCBSprite);
	if(FAILED(hr))
	{
		reset();
		return false;
	}

	float view[] =
	{
		2.0 / width, 0, 0, -1,
		0, -2.0 / height, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	ShaderConst cbConst;
	memcpy(cbConst.view, view, sizeof(view));
	m_pContext->UpdateSubresource(m_pCBConst, 0, NULL, &cbConst, 0, 0);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ID3D11BlendState* pBlendState = NULL;
	hr = m_pDevice->CreateBlendState(&blendStateDescription, &pBlendState);
	if (SUCCEEDED(hr))
	{
		 float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
		 m_pContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
		 pBlendState->Release();
	}

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable = TRUE;
	ID3D11RasterizerState *pRasterizerState = NULL;
	hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState);
	if (SUCCEEDED(hr))
	{
		m_pContext->RSSetState(pRasterizerState);
		pRasterizerState->Release();
	}

	return true;
}

void SvgaDx11CanvasPrivate::reset()
{
	for (QMap<QString, Dx11TextureInfo>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		iter.value().view->Release();
		iter.value().texture->Release();
	}
	m_textures.clear();

	if (m_pContext)
	{
		m_pContext->ClearState();
	}

	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = NULL;
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = NULL;
	}
}

bool SvgaDx11CanvasPrivate::begin()
{
	if (!m_pDevice || !m_pContext)
	{
		return false;
	}

	float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);

	return true;
}

void SvgaDx11CanvasPrivate::end()
{
	if (m_pSwapChain)
	{
		m_pSwapChain->Present(0, 0);
	}
}

void SvgaDx11CanvasPrivate::draw(DrawItem* item)
{
	ID3D11ShaderResourceView* texture = _getTexture(item->key, item->pix, item->clipPath, item->dynamic);
	if (!texture)
	{
		return;
	}

	float left = item->layout.left();
	float right = item->layout.right();
	float top = item->layout.top();
	float bottom = item->layout.bottom();
	ShaderVertex vertices[] =
	{
		{ left,  bottom, 0.0f,  0, 1 },
		{ right, bottom, 0.0f,  1, 1 },
		{ right, top, 0.0f,     1, 0 },
		{ left,  top, 0.0f,     0, 0 }
	};
	//m_pContext->UpdateSubresource(m_pVertexBuffer, 0, NULL, &vertices, 0, 0);
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT hr = m_pContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}
		ShaderVertex* data = (ShaderVertex*)mappedResource.pData;
		memcpy(data, vertices, sizeof(vertices));
		m_pContext->Unmap(m_pVertexBuffer, 0);
	}

	ShaderSprite cbSprite;
	float model[] =
	{
		item->transform.m11(), item->transform.m21(), 0, item->transform.dx(),
		item->transform.m12(), item->transform.m22(), 0, item->transform.dy(),
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	memcpy(cbSprite.model, model, sizeof(model));
	cbSprite.alpha = item->alpha;
	//m_pContext->UpdateSubresource(m_pCBSprite, 0, NULL, &cbSprite, 0, 0);
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT hr = m_pContext->Map(m_pCBSprite, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}
		ShaderSprite* data = (ShaderSprite*)mappedResource.pData;
		memcpy(data, &cbSprite, sizeof(cbSprite));
		m_pContext->Unmap(m_pCBSprite, 0);
	}

	m_pContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pContext->VSSetConstantBuffers(0, 1, &m_pCBConst);
	m_pContext->VSSetConstantBuffers(1, 1, &m_pCBSprite);
	m_pContext->PSSetShader(m_pPixelShader, NULL, 0);
	m_pContext->PSSetConstantBuffers(1, 1, &m_pCBSprite);
	m_pContext->PSSetShaderResources(0, 1, &texture);
	m_pContext->DrawIndexed(6, 0, 0);

	m_pContext->Flush();
}

ID3D11Texture2D* SvgaDx11CanvasPrivate::_loadImage(QImage& img, ID3D11ShaderResourceView** view)
{
	ID3D11Texture2D* texture = NULL;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = img.width();
	desc.Height = img.height();
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_pDevice->CreateTexture2D(&desc, NULL, &texture);
	if (FAILED(hr))
	{
		return NULL;
	}

	if (!_updateImage(img, texture))
	{
		texture->Release();
		return NULL;
	}

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = m_pDevice->CreateShaderResourceView(texture, &srvDesc, view);
	if (FAILED(hr))
	{
		texture->Release();
		return NULL;
	}

	//typedef HRESULT (WINAPI *pD3DXSaveTextureToFileW)(ID3D11DeviceContext*, ID3D11Resource*, D3DX11_IMAGE_FILE_FORMAT, LPCWSTR);
	//pD3DXSaveTextureToFileW pfnD3DXSaveTextureToFileW = (pD3DXSaveTextureToFileW)GetProcAddress(m_hD3d11XModule, "D3DX11SaveTextureToFileW");
	//if (pfnD3DXSaveTextureToFileW)
	//{
	//	pfnD3DXSaveTextureToFileW(m_pContext, texture, D3DX11_IFF_PNG, L"test.png");
	//}

	return texture;
}

bool SvgaDx11CanvasPrivate::_updateImage(QImage& img, ID3D11Texture2D* texture)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HRESULT hr = m_pContext->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	if (FAILED(hr))
	{
		return false;
	}

	unsigned int  linesize = mappedData.RowPitch;
	int stride = img.bytesPerLine();

	unsigned char* dest = reinterpret_cast<unsigned char*>(mappedData.pData);
	unsigned char* src = img.bits();

	if (linesize == stride)
	{
		memcpy(dest, src, stride * img.height());
	}
	else
	{
		for (int i = 0; i < img.height(); i++)
		{
			memcpy_s(dest, stride, src, stride);
			dest += mappedData.RowPitch;
			src += stride;
		}
	}

	m_pContext->Unmap(texture, 0);

	return true;
}

ID3D11ShaderResourceView* SvgaDx11CanvasPrivate::_getTexture(const QString& key, QPixmap& pix, const QString& clipPath, bool dynamic)
{
	SvgaPath clip;
	clip.setPath(clipPath);
	//return _loadPixmap(clip.clip(pix));

	Dx11TextureInfo info = m_textures.value(key);
	if (!info.texture)
	{
		info.texture = _loadImage(clip.clipAsImage(pix), &info.view);
		if (info.texture)
		{
			info.clipPath = clipPath;
			m_textures[key] = info;
		}
	}
	else if (info.clipPath != clipPath || info.dynamic != dynamic)
	{
		QImage image = clip.clipAsImage(pix);

		if (_updateImage(image, info.texture))
		{
			m_textures[key].clipPath = clipPath;
			m_textures[key].dynamic = dynamic;
		}
		else
		{
			return NULL;
		}
	}

	return info.view;
}

bool SvgaDx11CanvasPrivate::_compileShader(LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	ID3DBlob* errorBlob = NULL;

	typedef HRESULT (WINAPI *pD3DX11CompileFromMemory)(LPCSTR, SIZE_T, LPCSTR, CONST D3D10_SHADER_MACRO*, LPD3D10INCLUDE, LPCSTR, LPCSTR, UINT, UINT, ID3DX11ThreadPump*, ID3D10Blob**, ID3D10Blob**, HRESULT*);
	pD3DX11CompileFromMemory pfnD3DX11CompileFromMemory = (pD3DX11CompileFromMemory)GetProcAddress(m_hD3d11XModule, "D3DX11CompileFromMemory");
	if (pfnD3DX11CompileFromMemory)
	{
		hr = pfnD3DX11CompileFromMemory(hlsl, strlen(hlsl), NULL, NULL, NULL, szEntryPoint, szShaderModel, 1 << 11/*D3DCOMPILE_ENABLE_STRICTNESS*/, 0, NULL, ppBlobOut, &errorBlob, NULL);
	}
	else
	{
		return false;
	}

	if (FAILED(hr))
	{
		if(errorBlob)
		{
			char* error = (char*)errorBlob->GetBufferPointer();
			error;
			errorBlob->Release();
		}

		return false;
	}

	if (errorBlob)
	{
		errorBlob->Release();
	}

	return true;
}

SvgaDx11Canvas::SvgaDx11Canvas()
: m_ptr(new SvgaDx11CanvasPrivate(this))
{
	m_ptr->init();
}

SvgaDx11Canvas::~SvgaDx11Canvas()
{
	if (m_ptr)
	{
		m_ptr->release();
		delete m_ptr;
		m_ptr = NULL;
	}
}

void SvgaDx11Canvas::init(HWND parent)
{
	static bool bRegister = false;
	if (!bRegister)
	{
		bRegister = true;

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
		wcex.lpszClassName	= WIN_DX11_CLASS;
		wcex.hIconSm		= NULL;
		RegisterClassEx(&wcex);
	}

	Q_D(SvgaDx11Canvas);
	d->m_hwnd = CreateWindow(WIN_DX11_CLASS, WIN_DX11_NAME, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, GetModuleHandle(NULL), NULL);
}

void SvgaDx11Canvas::setPos(int x, int y)
{
	Q_D(SvgaDx11Canvas);
	MoveWindow(d->m_hwnd, x, y, d->m_videoWidth, d->m_videoHeight, FALSE);
}

void SvgaDx11Canvas::setVisible(bool visible)
{
	Q_D(SvgaDx11Canvas);
	ShowWindow(d->m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

void SvgaDx11Canvas::setVideoSize(int width, int height)
{
	Q_D(SvgaDx11Canvas);
	d->setup(width, height);
}

bool SvgaDx11Canvas::begin()
{
	Q_D(SvgaDx11Canvas);
	return d->begin();
}

void SvgaDx11Canvas::end()
{
	Q_D(SvgaDx11Canvas);
	d->end();
}

void SvgaDx11Canvas::draw(DrawItem* item)
{
	Q_D(SvgaDx11Canvas);
	d->draw(item);
}
