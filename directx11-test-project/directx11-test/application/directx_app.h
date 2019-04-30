#pragma once

#include <application/windows_app.h>
#include <application/directx_settings.h>
#include <time/timer.h>

namespace xtest {
namespace application {

	class DirectxApp : public WindowsApp
	{
	public:

		DirectxApp(HINSTANCE instance, const WindowSettings& windowSettings, const DirectxSettings& directxSettings, uint32 fps = 60);
		virtual ~DirectxApp() {};

		DirectxApp(DirectxApp&&) = delete;
		DirectxApp(const DirectxApp&) = delete;
		DirectxApp& operator=(DirectxApp&&) = delete;
		DirectxApp& operator=(const DirectxApp&) = delete;


		virtual void Init() override;
		void Run();
		void SetPause(bool wantToPause);
		bool IsPaused() const;
		void SetViewport(uint32 x, uint32 y, uint32 width, uint32 height);

		virtual void OnMinimized() override;
		virtual void OnResized() override;
		virtual void OnActive() override;
		virtual void OnInactive() override;

		const DirectxSettings& GetDirectXSettings() const;

		virtual void UpdateScene(float deltaSeconds) = 0;
		virtual void RenderScene() = 0;
		time::Timer& GetGlobalTimer();

	protected:

		void InitDirectX();
		void ResizeBuffers();

		time::Timer m_timer;
		Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_d3dAnnotation;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthBufferView;

	private:
		
		void UpdateFrameStats(float frameTimeSec) const;
		void CreateDepthStencilBuffer();
		void FixedRenderingLoop();

		DirectxSettings m_directxSettings;
		bool m_isActive;
		uint32 m_fixedRenderingLoopFrequency;
	};

} // application 
} // xtest

