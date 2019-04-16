#pragma once

#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>

namespace xtest {
namespace demo {

	class BoxDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
	{
	public:

		struct VertexIn {
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT4 color;
		};

		struct PerObjectCB
		{
			DirectX::XMFLOAT4X4 WVP;
		};

		BoxDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
		~BoxDemoApp();

		BoxDemoApp(BoxDemoApp&&) = delete;
		BoxDemoApp(const BoxDemoApp&) = delete;
		BoxDemoApp& operator=(BoxDemoApp&&) = delete;
		BoxDemoApp& operator=(const BoxDemoApp&) = delete;


		virtual void Init() override;
		virtual void OnResized() override;
		virtual void UpdateScene(float deltaSeconds) override;
		virtual void RenderScene() override;

		virtual void OnWheelScroll(input::ScrollStatus scroll) override;
		virtual void OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos) override;
		virtual void OnKeyStatusChange(input::Key key, const input::KeyStatus& status) override;

	private:

		void InitMatrices();
		void InitShaders();
		void InitBuffers();
		void InitRasterizerState();

		DirectX::XMFLOAT4X4 m_viewMatrix;
		DirectX::XMFLOAT4X4 m_worldMatrix;
		DirectX::XMFLOAT4X4 m_projectionMatrix;

		camera::SphericalCamera m_camera;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

	};

} // demo
} // xtest

