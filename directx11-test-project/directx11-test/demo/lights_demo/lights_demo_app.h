#pragma once

#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_format.h>

namespace xtest {
	namespace demo {

		class LightDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
		{
		public:

			struct Material 
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
			};

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_inverseTranspose;
				DirectX::XMFLOAT4X4 WVP;
				Material material;
			};

			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 dirW;
				float _explicit_pad_;
			};

			struct PointLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 posW;
				float range;
				DirectX::XMFLOAT3 attenuation;
				float _explicit_pad_;
			};

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
			};

			struct PerFrameCB {
				DirectionalLight dirLigth;
				PointLight pLight;
				SpotLight sLight;
				DirectX::XMFLOAT3 eyePos;
				float _explicit_pad_;
			};

			struct ObjectToDraw {
				Material material;
				DirectX::XMFLOAT4X4 worldMatrix;
				UINT indeces;
				Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
			};

			

			LightDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~LightDemoApp();

			LightDemoApp(LightDemoApp&&) = delete;
			LightDemoApp(const LightDemoApp&) = delete;
			LightDemoApp& operator=(LightDemoApp&&) = delete;
			LightDemoApp& operator=(const LightDemoApp&) = delete;


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
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;


			std::size_t m_objectsNumber;
			std::vector<ObjectToDraw> m_objects;

			DirectionalLight m_dirLight;
			PointLight m_pLight;
			SpotLight m_sLight;

			std::map<std::string, Material> m_crateMaterials;
			DirectX::XMFLOAT4X4 m_crateWorldMatrix;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_crateVertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_crateIndexBuffer;
			std::map<std::string, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_crateConstantBufferMap;
			std::map<std::string, xtest::mesh::GPFMesh::MeshDescriptor> m_crateMeshDescriptorMap;


			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsLightBuffer;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		};

	} // demo
} // xtest