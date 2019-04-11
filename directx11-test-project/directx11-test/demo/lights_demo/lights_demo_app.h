#pragma once

#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_format.h>

namespace xtest {
	namespace demo {

		class LightsDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
		{

		public:


			struct Material
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
			};

			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 dirW;
				float _explicit_pad_;
			} m_directionalLight;

			struct PointLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 posW;
				float range;
				DirectX::XMFLOAT3 attenuation;
				float _explicit_pad_;
			} m_pointLight;

			struct SpotLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 posW;
				float range;
				DirectX::XMFLOAT3 dirW;
				float spot;
				DirectX::XMFLOAT3 attenuation;
				float _explicit_pad_;
			} m_spotLight;

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_inverseTranspose;
				DirectX::XMFLOAT4X4 WVP;
				Material material;
			};

			struct PerFrameCB
			{
				DirectionalLight directionalLight;
				PointLight pointLight;
				SpotLight spotLight;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad;
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
			void InitLights();
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
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightsBuffer;

			struct Model {
				mesh::MeshData Data;
				uint32 VerticesCount;
				uint32 IndicesCount;
				Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
				Material material;
				Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
				DirectX::XMFLOAT4X4 WorldMatrix, RotationMatrix;
				float rotationSpeed;
			};

			typedef std::map<std::string, Model> ModelsMap;
			ModelsMap m_models;

			struct Crate {
				xtest::mesh::GPFMesh Data;
				uint32 VerticesCount;
				uint32 IndicesCount;
				Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
				std::map<std::string, Microsoft::WRL::ComPtr<ID3D11Buffer>> ConstantBuffers;
				std::map<std::string, Material> materials;
				Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
				DirectX::XMFLOAT4X4 WorldMatrix, RotationMatrix, ScaleMatrix;
				float rotationSpeed;
			} m_crate;
		};

	} // demo
} // xtest

