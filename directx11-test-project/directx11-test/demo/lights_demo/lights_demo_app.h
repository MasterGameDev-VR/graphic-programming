#pragma once

#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>

namespace xtest {
	namespace demo {

		class LightsDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
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

			LightsDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~LightsDemoApp();

			LightsDemoApp(LightsDemoApp&&) = delete;
			LightsDemoApp(const LightsDemoApp&) = delete;
			LightsDemoApp& operator=(LightsDemoApp&&) = delete;
			LightsDemoApp& operator=(const LightsDemoApp&) = delete;


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
			void InitMeshes();

			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

			struct Model {
				std::vector<VertexIn> Vertices;
				uint32 VerticesCount;
				std::vector<uint32> Indices;
				uint32 IndicesCount;
				Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
				Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
				Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
				Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
				DirectX::XMFLOAT4X4 WorldMatrix;
			};

			typedef std::map<std::string, Model> ModelsMap;
			ModelsMap m_meshes;
		};

	} // demo
} // xtest

