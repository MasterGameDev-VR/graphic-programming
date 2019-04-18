#pragma once

#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>

#define POINTNUMBER 5


namespace xtest {
	namespace demo {


		/*
			Use F1, F2, F3 key to switch on/off each light individually
			Use Spacebar to pause lights motion
			Use F key to reframe the camera to the origin
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to zoom in/out
		*/

		class TexturesDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
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

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_inverseTraspose;
				DirectX::XMFLOAT4X4 WVP;
				DirectX::XMFLOAT4X4 textureMatrix;
				Material material;
			};

			struct PerFrameCB
			{
				DirectionalLight dirLight;
				std::array<PointLight, POINTNUMBER> pointLights;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
			};

			struct RarelyChangedCB
			{
				int32 useDirLight;
				int32 usePointLight;
				int32 useSpotLight;
				int32 _explicit_pad_;
			};


			struct Renderable
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 textureMatrix;
				std::string materialKey;
				std::string textureViewKey;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectCB;
			};

			struct Mesh {
				mesh::MeshData meshData;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
			};

			struct ObjectGroup {
				std::vector<Renderable> objects;
				Mesh mesh;
			};


			struct GPFRenderable
			{
				struct ShapeAttributes
				{
					Material material;
					Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectCB;
				};

				mesh::GPFMesh mesh;
				std::map<std::string, ShapeAttributes> shapeAttributeMapByName;
				DirectX::XMFLOAT4X4 W;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
			};

			struct TexturePack {
				Microsoft::WRL::ComPtr<ID3D11Resource> texture;
				Microsoft::WRL::ComPtr<ID3D11Resource> normalMap;
			};

			struct TexturePackView {
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapView;
			};


			TexturesDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~TexturesDemoApp();

			TexturesDemoApp(TexturesDemoApp&&) = delete;
			TexturesDemoApp(const TexturesDemoApp&) = delete;
			TexturesDemoApp& operator=(TexturesDemoApp&&) = delete;
			TexturesDemoApp& operator=(const TexturesDemoApp&) = delete;


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
			void InitRenderable();
			void InitLights();
			void InitRasterizerState();
			void InitTextures();
			void InitMaterials();
			void InitObjects();

			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			DirectionalLight m_dirLight;
			std::array<PointLight, POINTNUMBER> m_pointLights;
			RarelyChangedCB m_lightsControl;
			bool m_isLightControlDirty;
			bool m_stopLights;

			size_t m_objectsNumber;

			std::vector<ObjectGroup> m_objectsToDrawByGroup;

			std::map<std::string, Material> m_materialMap;
			std::map<std::string, TexturePackView> m_texturePackViewMap;
			std::vector<TexturePack> m_texturePacks;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> m_textureSampler;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dPerFrameCB;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dRarelyChangedCB;
			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

		};

	} // demo
} // xtest
