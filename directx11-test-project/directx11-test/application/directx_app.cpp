#include "stdafx.h"
#include "directx_app.h"
#include <render/render_macros.h>
#include <render/directx_backend.h>
#include <service/locator.h>


using xtest::application::DirectxApp;
using xtest::application::WindowSettings;
using xtest::application::DirectxSettings;
using Microsoft::WRL::ComPtr;


DirectxApp::DirectxApp(HINSTANCE instance, const WindowSettings& windowSettings, const DirectxSettings& directxSettings, uint32 fps /*=60*/)
	: WindowsApp(instance, windowSettings)
	, m_timer()
	, m_d3dDevice()
	, m_d3dContext()
	, m_swapChain()
	, m_backBufferView()
	, m_depthBuffer()
	, m_depthBufferView()
	, m_directxSettings(directxSettings)
	, m_isActive(true)
	, m_fixedRenderingLoopFrequency(fps)
	, m_resourceLoader()
{}


void DirectxApp::Init()
{
	WindowsApp::Init();
	InitDirectX();

	service::Locator::ProvideD3DDevice(m_d3dDevice.Get());
	service::Locator::ProvideD3DContext(m_d3dContext.Get());
	service::Locator::ProvideResourceLoader(&m_resourceLoader);
}


void DirectxApp::InitDirectX()
{
	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// we support only directx 11
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 , D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_12_0 };

	// create device and context
	XTEST_D3D_CHECK(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		featureLevels,
		sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&m_d3dDevice,
		nullptr,
		&m_d3dContext
	));


	// annotation interface used for setting debug markers and regions
	XTEST_D3D_CHECK(m_d3dContext->QueryInterface(__uuidof(m_d3dAnnotation), &m_d3dAnnotation));


	// select the best supported mode by the primary screen
	std::vector<DXGI_MODE_DESC> modes = render::BestMatchOutputModes(
		render::PrimaryOutput(m_d3dDevice),
		GetCurrentWidth(),
		GetCurrentHeight(),
		render::OutputModeOrder::high_refresh_first
	);

	XTEST_ASSERT(modes.size() > 0, L"No compatible output modes have been found.");


	// swap chain creation
	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferDesc.Width = modes[0].Width;
	swapDesc.BufferDesc.Height = modes[0].Height;
	swapDesc.BufferDesc.RefreshRate = modes[0].RefreshRate; // mode with the faster refresh rate
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.SampleDesc.Count = 1;   //no MSAA
	swapDesc.SampleDesc.Quality = 0; //no MSAA
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = static_cast<UINT>(m_directxSettings.buffering);
	swapDesc.OutputWindow = GetMainWindow();
	swapDesc.Windowed = true; // we'll treat the full screen as a borderless full screen window
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;

	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ComPtr<IDXGIFactory> dxgiFactory;
	XTEST_D3D_CHECK(m_d3dDevice.As(&dxgiDevice));
	XTEST_D3D_CHECK(dxgiDevice->GetAdapter(&dxgiAdapter));
	XTEST_D3D_CHECK(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));
	XTEST_D3D_CHECK(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapDesc, &m_swapChain));

	// create the back buffer view, the back buffer was already created by the swap chain
	ComPtr<ID3D11Texture2D> backBuffer;
	XTEST_D3D_CHECK(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)); // using DXGI_SWAP_EFFECT_DISCARD we can only access the first buffer
	XTEST_D3D_CHECK(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, &m_backBufferView));

	//disable automatic alt+enter fullscreen switch, we are using borderless fullscreen
	dxgiFactory->MakeWindowAssociation(GetMainWindow(), DXGI_MWA_NO_ALT_ENTER);


	CreateDepthStencilBuffer();
	SetViewport(0, 0, GetCurrentWidth(), GetCurrentHeight());

}


void DirectxApp::CreateDepthStencilBuffer()
{
	// release any previous references
	m_depthBuffer.Reset();
	m_depthBufferView.Reset();

	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = GetCurrentWidth();
	depthDesc.Height = GetCurrentHeight();
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;		// no MSAA
	depthDesc.SampleDesc.Quality = 0;	// no MSAA
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	// create the depth buffer and its view
	XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, &m_depthBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &m_depthBufferView));

	// bind the views to the rendering pipeline (output merger state)
	m_d3dContext->OMSetRenderTargets(1, m_backBufferView.GetAddressOf(), m_depthBufferView.Get());
}


void DirectxApp::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
{
	m_viewport.TopLeftX = FLOAT(x);
	m_viewport.TopLeftY = FLOAT(y);
	m_viewport.Width = FLOAT(width);
	m_viewport.Height = FLOAT(height);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	// set the viewport to the rasterizer stage
	m_d3dContext->RSSetViewports(1, &m_viewport);
}


void DirectxApp::ResizeBuffers()
{

	unsigned currentWidth = GetCurrentWidth();
	unsigned currentHeight = GetCurrentHeight();

	//release the back buffer view reference
	m_backBufferView.Reset();

	// resize the swap chain
	XTEST_D3D_CHECK(m_swapChain->ResizeBuffers(
		static_cast<UINT>(m_directxSettings.buffering),
		currentWidth,
		currentHeight,
		DXGI_FORMAT_UNKNOWN, // in this case unknown preserve the previous format
		0));

	//update the views
	ComPtr<ID3D11Texture2D> backBuffer;
	XTEST_D3D_CHECK(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, &m_backBufferView));

	CreateDepthStencilBuffer();
	SetViewport(0, 0, currentWidth, currentHeight);
}


void DirectxApp::Run()
{
	FixedRenderingLoop();
}


xtest::time::Timer& DirectxApp::GetGlobalTimer()
{
	return m_timer;
}


void DirectxApp::FixedRenderingLoop()
{
	Show();
	m_timer.Reset();

	const float k_fixedTimeStepSec = 1.f / float(m_fixedRenderingLoopFrequency);
	time::TimePoint startTime;
	time::TimePoint expectedFrameEndTime;
	time::TimePoint currentTime;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (m_isActive)
		{
			startTime = time::TimePoint::Now();
			expectedFrameEndTime = startTime + time::TimeSpan(k_fixedTimeStepSec);

			// we advance only in fixed time steps
			m_timer.Update(time::TimeSpan(k_fixedTimeStepSec));
			UpdateScene(m_timer.DeltaTime().Seconds());
			RenderScene();

			// if we were too fast we have to wait but we can't rely on Sleep to be precise,
			// so each time we sleep only for a third of the needed time and repeat the check.
			currentTime = time::TimePoint::Now();
			while (currentTime < expectedFrameEndTime)
			{
				// note that we don't sleep when the remaining time is less than 3ms,
				// the integer division will round it to zero.
				Sleep(DWORD((expectedFrameEndTime - currentTime).Millis()) / 3);
				currentTime = time::TimePoint::Now();
			}

			UpdateFrameStats((time::TimePoint::Now() - startTime).Seconds());
		}
		else
		{
			Sleep(100);
		}
	}
}

void DirectxApp::SetPause(bool wantToPause)
{
	XTEST_DEBUG_LOG("The application is now " << (wantToPause ? "paused" : "resumed"));
	m_isActive = !wantToPause;
	m_timer.SetPause(wantToPause);
}

bool DirectxApp::IsPaused() const
{
	return !m_isActive;
}


void DirectxApp::OnMinimized()
{
	SetPause(true);
}


void DirectxApp::OnResized()
{
	XTEST_DEBUG_LOG("The application window has been resized, new dimensions:(" << GetCurrentWidth() << ", " << GetCurrentHeight() << ")");
	ResizeBuffers();
}


void DirectxApp::OnActive()
{
	SetPause(false);
}


void DirectxApp::OnInactive()
{
	SetPause(true);
}


const DirectxSettings& DirectxApp::GetDirectXSettings() const
{
	return m_directxSettings;
}


void DirectxApp::UpdateFrameStats(float frameTimeSec) const
{
	if (m_directxSettings.showFrameStats)
	{
		static time::TimePoint previousTime = time::TimePoint::Now();

		// we display the current frameTime each 0.1 sec
		if ((time::TimePoint::Now() - previousTime) > time::TimeSpan(0.1f))
		{
			std::wostringstream statString;
			statString.precision(6);
			statString << GetWindowSettings().title << L" - FPS:" << std::round(1.f / frameTimeSec) << L" - " << frameTimeSec * 1000.f << L"ms";
			SetWindowTextW(GetMainWindow(), statString.str().c_str());

			previousTime = time::TimePoint::Now();
		}
	}
}

