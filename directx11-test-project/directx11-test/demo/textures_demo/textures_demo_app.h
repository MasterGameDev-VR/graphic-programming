#pragma once

#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>


namespace xtest {
	namespace demo {


		/*
			Use F1, F2, F3 key to switch on/off each light individually
			Use F4, F5, F6 key to switch on/off each texture mapping type
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
				DirectX::XMFLOAT4X4 TexcoordMatrix;
				Material material;
				DirectX::XMFLOAT4X4 MovingTexcoordMatrix;
				int32 hasMovingTexture;
				DirectX::XMFLOAT3 _explicit_pad_;
			};

			struct PerFrameCB
			{
				DirectionalLight dirLight;
				std::array<PointLight, 5> pointLights;
				SpotLight spotLight;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
			};

			struct RarelyChangedCB
			{
				int32 useDirLight;
				int32 usePointLight;
				int32 useSpotLight;
				int32 useDiffuseTexture;
				int32 useNormalTexture;
				int32 useGlossTexture;
				int32 _explicit_pad_1_;
				int32 _explicit_pad_2_;
			};

			struct Renderable
			{
				mesh::MeshData mesh;
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 textureMatrix;
				DirectX::XMFLOAT4X4 movingTextureMatrix;
				Material material;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectCB;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Resource> texture;
				Microsoft::WRL::ComPtr<ID3D11Resource> movingTexture;
				Microsoft::WRL::ComPtr<ID3D11Resource> normalMap;
				Microsoft::WRL::ComPtr<ID3D11Resource> glossMap;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> movingTextureView;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapView;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glossMapView;
				bool hasMovingTexture;
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
			void InitTextures();
			void InitShaders();
			void InitRenderable();
			void InitLights();
			void InitRasterizerState();


			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			DirectionalLight m_dirLight;
			SpotLight m_spotLight;
			std::array<PointLight, 5> m_pointLights;
			RarelyChangedCB m_controls;
			bool m_isControlsDirty;
			bool m_stopLights;

			std::vector<Renderable> m_renderables;

			// Texture
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


