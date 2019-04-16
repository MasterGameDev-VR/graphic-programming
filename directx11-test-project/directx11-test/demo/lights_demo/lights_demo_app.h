#pragma once

#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>

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
				float _explicit_pad;
			};

			struct PointLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 posW;
				float range;
				DirectX::XMFLOAT3 attenuation;
				float _explicit_pad;
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
				float _explicit_pad;
			};

			struct VertexIn {
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT3 normal;
				DirectX::XMFLOAT3 tangent;
				DirectX::XMFLOAT2 uv;
			};

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_INV_T;
				DirectX::XMFLOAT4X4 WVP;
				Material material;
			};

			struct PerFrameCB
			{
				DirectionalLight dirlight;
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
			void InitShaders();
			void InitBuffers();
			void InitRasterizerState();
			void CreateBuffers(int);
			void BindPerMesh(int, UINT, UINT, UINT, UINT);
			void MapPerMesh(int, DirectX::XMMATRIX&, DirectX::XMMATRIX&);
			void GenerateCrate(uint32, uint32, uint32, uint32, uint32, xtest::mesh::GPFMesh& crate);
			void TransformMesh(DirectX::XMMATRIX& transformMatrix, std::vector<xtest::mesh::MeshData::Vertex>& vertices);

			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_worldMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			std::vector<xtest::mesh::MeshData> m_meshesToDraw;
			std::vector<UINT> m_indexCountPerMesh;
			std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_vertexBuffers;
			std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_indexBuffers;
			std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> m_vsConstantBufferObjects;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBufferFrame;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
			std::vector<Material> materialPerMesh;
			std::vector<D3D11_BUFFER_DESC> vertexBufferDescs;
			std::vector<D3D11_BUFFER_DESC> indexBufferDescs;
			std::vector<D3D11_BUFFER_DESC> vsConstantBufferDescs;
			std::vector<D3D11_SUBRESOURCE_DATA> vertexInitData;
			std::vector<D3D11_SUBRESOURCE_DATA> indexInitData;
			size_t meshesNumber = 9;
			bool directional = true;
			bool point = true;
			bool spot = true;
		};

	} // demo
} // xtest

