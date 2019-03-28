#include "stdafx.h"
#include "directx_app.h"
#include <render/render_macros.h>
#include <render/directx_backend.h>

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
{}


void DirectxApp::Init()
{
	WindowsApp::Init();
	InitDirectX();
}


void DirectxApp::InitDirectX()
{
	// DONE
	// TODO: Creare un device e un devicecontext:
	// 1. crea un ID3D11Device e ID3D11DeviceContext utilizzando la funzione D3D11CreateDevice 
	// 2. salvali nelle variabili m_d3dDevice e m_d3dContext di classe, attenzione a come si utilizza 
	//    un ComPtr

	D3D_FEATURE_LEVEL pFeatureLevels[] =
	{
		 D3D_FEATURE_LEVEL_11_0,
		 D3D_FEATURE_LEVEL_10_1,
		 D3D_FEATURE_LEVEL_10_0,
		 D3D_FEATURE_LEVEL_9_3,
		 D3D_FEATURE_LEVEL_9_2,
		 D3D_FEATURE_LEVEL_9_1
	};

	XTEST_D3D_CHECK(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		pFeatureLevels,
		ARRAYSIZE(pFeatureLevels),
		D3D11_SDK_VERSION,
		m_d3dDevice.GetAddressOf(),
		nullptr,
		m_d3dContext.GetAddressOf()
	));


	// DONE
	//TODO: Decommenta questo pezzo di codice una volta che m_d3dDevice e m_d3dDeviceContext sono
	//      stati creati, è stato commentato cosicchè se farai il run del programma senza aver iniziato ad
	//      implementare i vari "todo" quest'ultimo mostri comunque una finestra vuota senza andare in errore
	//
	// [decommenta da qui -->]

	// select the best supported mode by the primary screen
	std::vector<DXGI_MODE_DESC> modes = render::BestMatchOutputModes(
		render::PrimaryOutput(m_d3dDevice), 
		GetCurrentWidth(),
		GetCurrentHeight(),
		render::OutputModeOrder::high_refresh_first
	);

	XTEST_ASSERT(modes.size() > 0, L"No compatible output modes have been found.");

	// [<-- decommenta fino a qui]


	// DONE
	// TODO: Creare la swap chain:
	// 1. riempire la struttura DXGI_SWAP_CHAIN_DESC, per with, height e refresh utilizza
	//	  modes[0].Width, modes[0].Height e modes[0].RefreshRate calcolati dal codice poco sopra (da riabilitare)
	//	  utilizza la funzione GetMainWindow() di WindowsApp (questo framework) per settare l'outputWindow
	// 2. crea la swap chain utilizzando il metodo CreateSwapChain di IDXGIFactory, puoi ricevere un'istanza di
	//	  IDXGIFactory attraverso il metodo GetParent di IDXGIAdapter il quale può essere raggiunto attraverso un IDXGIDevice,
	//    puoi capire come ricevere un IDXGIAdapter e un IDXGIDevice studiando l'implementazione della funzione render::PrimaryOutput
	//    di questo framework
	// 3. salva la swap chain nella variabile m_swapChain di classe

	DXGI_SWAP_CHAIN_DESC swapDesc;
	swapDesc.BufferDesc.Width = modes[0].Width;
	swapDesc.BufferDesc.Height = modes[0].Height;
	swapDesc.BufferDesc.RefreshRate = modes[0].RefreshRate;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = static_cast<UINT>(m_directxSettings.buffering);
	swapDesc.OutputWindow = GetMainWindow();
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;

	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ComPtr<IDXGIFactory> dxgiFactory;

	XTEST_D3D_CHECK(m_d3dDevice.As(&dxgiDevice));
	XTEST_D3D_CHECK(dxgiDevice->GetAdapter(&dxgiAdapter));
	XTEST_D3D_CHECK(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));
	XTEST_D3D_CHECK(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapDesc, &m_swapChain));


	// DONE
	// TODO: Crea una render target view del back buffer contenuto nella swap chain
	// 1. richiedi alla swap chain un riferimento al backbuffer tramite il metodo GetBuffer della swap chain
	//
	// hint: studia il metodo ResizeBuffer() di questa classe per conoscere come recuperare il rifermento al 
	//       back buffer
	//
	// 2. crea la view attraverso il metodo CreateRenderTargetView di ID3D11Device e storicizzala in m_backBufferView

	ComPtr<ID3D11Texture2D> backBuffer;
	XTEST_D3D_CHECK(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, &m_backBufferView));
	
	CreateDepthStencilBuffer();
	SetViewport(0, 0, GetCurrentWidth(),GetCurrentHeight());

}


void DirectxApp::CreateDepthStencilBuffer()
{
	// release any previous references
	m_depthBuffer.Reset();
	m_depthBufferView.Reset();


	// DONE
	// TODO:crea un depth/stencil buffer:
	// 1. Riempi la struttura D3D11_TEXTURE2D_DESC, ricorda che puoi conoscere la larghezza e altezza della finestra
	//    tramite i metodi GetCurrentWidth e GetCurrentHeight di WindowsApp. Ricorda che il formato per un back buffer 
	//    è molto speciale, per le nostre esercitazioni DXGI_FORMAT_D24_UNORM_S8_UINT è più che sufficiente. Ricorda inolte che 
	//    la texture che stai creando andrà utilizzata come depth stencil buffer, dunque fai attenzione al parametro BindFlags 
	// 2. Crea la texture utilizzando CreateTexture2D di ID3D11Device e salvala nel membro m_depthBuffer
	// 3. Crea la view di tale texture attraverso CreateDepthStencilView di ID3D11Device e salvala in m_depthBufferView

	D3D11_TEXTURE2D_DESC depthStencilBuff;
	depthStencilBuff.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBuff.Width = GetCurrentWidth();
	depthStencilBuff.Height = GetCurrentHeight();
	depthStencilBuff.ArraySize = 1;
	depthStencilBuff.MipLevels = 1;
	depthStencilBuff.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBuff.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBuff.CPUAccessFlags = 0;
	depthStencilBuff.SampleDesc.Count = 1;
	depthStencilBuff.SampleDesc.Quality = 0;
	depthStencilBuff.MiscFlags = 0;

	XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&depthStencilBuff, nullptr, &m_depthBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &m_depthBufferView));


	// DONE
	// TODO:
	// aggancia la depth buffer view e la back buffer view create in precedenza all'OutputMerger utilizzando la funzione
	// OMSetRenderTargets di ID3D11DeviceContext

	m_d3dContext->OMSetRenderTargets(1, m_backBufferView.GetAddressOf(), m_depthBufferView.Get());

}


void DirectxApp::SetViewport(uint32 x, uint32 y, uint32 width, uint32 height)
{
	D3D11_VIEWPORT viewportDesc;
	viewportDesc.TopLeftX = FLOAT(x);
	viewportDesc.TopLeftY = FLOAT(y);
	viewportDesc.Width = FLOAT(width);
	viewportDesc.Height = FLOAT(height);
	viewportDesc.MinDepth = 0.f;
	viewportDesc.MaxDepth = 1.f;

	// DONE
	// TODO:
	// setta la viewport del RasterizerStage attraverslo la funzione RSSetViewports di ID3D11DeviceContext
	// utilizzando la struttura creata poco sopra

	m_d3dContext->RSSetViewports(1, &viewportDesc);

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
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
				Sleep( DWORD((expectedFrameEndTime - currentTime).Millis()) / 3);
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

