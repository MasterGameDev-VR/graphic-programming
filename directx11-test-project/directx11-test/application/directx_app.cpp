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
	// TODO: Creare un device e un devicecontext:
	// 1. crea un ID3D11Device e ID3D11DeviceContext utilizzando la funzione D3D11CreateDevice 
	// 2. salvali nelle variabili m_d3dDevice e m_d3dContext di classe, attenzione a come si utilizza 
	//    un ComPtr

	
	//TODO: Decommenta questo pezzo di codice una volta che m_d3dDevice e m_d3dDeviceContext sono
	//      stati creati, � stato commentato cosicch� se farai il run del programma senza aver iniziato ad
	//      implementare i vari "todo" quest'ultimo mostri comunque una finestra vuota senza andare in errore
	//
	/* [decommenta da qui -->]

	// select the best supported mode by the primary screen
	std::vector<DXGI_MODE_DESC> modes = render::BestMatchOutputModes(
		render::PrimaryOutput(m_d3dDevice), 
		GetCurrentWidth(),
		GetCurrentHeight(),
		render::OutputModeOrder::high_refresh_first
	);

	XTEST_ASSERT(modes.size() > 0, L"No compatible output modes have been found.");

	[<-- decommenta fino a qui] */



	// TODO: Creare la swap chain:
	// 1. riempire la struttura DXGI_SWAP_CHAIN_DESC, per with, height e refresh utilizza
	//	  modes[0].Width, modes[0].Height e modes[0].RefreshRate calcolati dal codice poco sopra (da riabilitare)
	//	  utilizza la funzione GetMainWindow() di WindowsApp (questo framework) per settare l'outputWindow
	// 2. crea la swap chain utilizzando il metodo CreateSwapChain di IDXGIFactory, puoi ricevere un'istanza di
	//	  IDXGIFactory attraverso il metodo GetParent di IDXGIAdapter il quale pu� essere raggiunto attraverso un IDXGIDevice,
	//    puoi capire come ricevere un IDXGIAdapter e un IDXGIDevice studiando l'implementazione della funzione render::PrimaryOutput
	//    di questo framework
	// 3. salva la swap chain nella variabile m_swapChain di classe

	

	// TODO: Crea una render target view del back buffer contenuto nella swap chain
	// 1. richiedi alla swap chain un riferimento al backbuffer tramite il metodo GetBuffer della swap chain
	//
	// hint: studia il metodo ResizeBuffer() di questa classe per conoscere come recuperare il rifermento al 
	//       back buffer
	//
	// 2. crea la view attraverso il metodo CreateRenderTargetView di ID3D11Device e storicizzala in m_backBufferView

	
	CreateDepthStencilBuffer();
	SetViewport(0, 0, GetCurrentWidth(),GetCurrentHeight());

}


void DirectxApp::CreateDepthStencilBuffer()
{
	// release any previous references
	m_depthBuffer.Reset();
	m_depthBufferView.Reset();


	// TODO:crea un depth/stencil buffer:
	// 1. Riempi la struttura D3D11_TEXTURE2D_DESC, ricorda che puoi conoscere la larghezza e altezza della finestra
	//    tramite i metodi GetCurrentWidth e GetCurrentHeight di WindowsApp. Ricorda che il formato per un back buffer 
	//    � molto speciale, per le nostre esercitazioni DXGI_FORMAT_D24_UNORM_S8_UINT � pi� che sufficiente. Ricorda inolte che 
	//    la texture che stai creando andr� utilizzata come depth stencil buffer, dunque fai attenzione al parametro BindFlags 
	// 2. Crea la texture utilizzando CreateTexture2D di ID3D11Device e salvala nel membro m_depthBuffer
	// 3. Crea la view di tale texture attraverso CreateDepthStencilView di ID3D11Device e salvala in m_depthBufferView



	// TODO:
	// aggancia la depth buffer view e la back buffer view create in precedenza all'OutputMerger utilizzando la funzione
	// OMSetRenderTargets di ID3D11DeviceContext

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

	// TODO:
	// setta la viewport del RasterizerStage attraverslo la funzione RSSetViewports di ID3D11DeviceContext
	// utilizzando la struttura creata poco sopra

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

