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
			Use Spacebar to pause lights motion
			Use F key to reframe the camera to the origin
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to zoom in/out
		*/

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
				Material material;
				int32 usesNormalMapTexture;
				int32 usesTwoColorMapTextures;
				int32 _explicit_pad_1_;
				int32 _explicit_pad_2_;
			};
			/*
			struct PerObjectTextureCB 
			{
				int32 usesNormalMapTexture;
				int32 usesTwoColorMapTextures;
				int32 _explicit_pad_1_;
				int32 _explicit_pad_2_;
			};*/
			

			struct PerFrameCB
			{
				DirectionalLight dirLight;
				PointLight pointLight0;
				PointLight pointLight1;
				PointLight pointLight2;
				PointLight pointLight3;
				PointLight pointLight4;
				SpotLight spotLight0;
				SpotLight spotLight1;
				SpotLight spotLight2;
				DirectX::XMFLOAT3 eyePosW;
				float translateValue;
			};
			/*
			struct PerFrameTextureCB 
			{
				DirectX::XMFLOAT4X4 texCoordMatrix;
				float translateValue;
				float _explicit_pad_1_;
				float _explicit_pad_2_;
				float _explicit_pad_3_;

			};
			*/

			struct RarelyChangedCB
			{
				int32 useDirLight;
				int32 usePointLight;
				int32 useSpotLight;
				int32 _explicit_pad_;
			};

			struct RarelyChangedTextureCB
			{
				int32 useColorTextureMap;
				int32 useNormalTextureMap;
				int32 useGlossTextureMap;
				int32 _explicit_pad_;
			};
	

			struct Renderable
			{
				mesh::MeshData mesh;
				DirectX::XMFLOAT4X4 W;
				Material material;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectCB;
				//Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectTextureCB;

				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceDiffText;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewDiffText;
				//texture that moves resource and resource view
				Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceMotionDiffText;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewMotionDiffText;
				//----------
				Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceNormalMapText;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewNormalMapText;
				Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceGlossMapText;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewGlossMapText;
			};

			
			struct GPFRenderable
			{
				struct ShapeAttributes
				{
					Material material;
					Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectCB;
					//Microsoft::WRL::ComPtr<ID3D11Buffer> d3dPerObjectTextureCB;
					Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceDiffText;
					Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewDiffText;
					Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceNormalMapText;
					Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewNormalMapText;
					Microsoft::WRL::ComPtr<ID3D11Resource> d3dResourceGlossMapText;
					Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderResourceViewGlossMapText;
				};

				mesh::GPFMesh mesh;
				std::map<std::string, ShapeAttributes> shapeAttributeMapByName;
				DirectX::XMFLOAT4X4 W;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3dIndexBuffer;
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
			void InitRenderable();
			void InitSamplerState();
			void InitLights();
			void InitTexturesControls();
			void InitRasterizerState();


			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			DirectionalLight m_dirLight;
			std::vector<PointLight> m_pointLights;
			std::vector<SpotLight> m_spotLights;

			RarelyChangedCB m_lightsControl;
			bool m_isLightControlDirty;
			bool m_stopLights;

			RarelyChangedTextureCB m_texturesControl;
			bool m_isTexturesControlDirty;

			bool m_isTheFirstTextureMovementUpdate;

			Renderable m_sphere;
			Renderable m_plane;
			Renderable m_torus;
			Renderable m_box;
			GPFRenderable m_crate;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dPerFrameCB;
			//Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dPerFrameTextureCB;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dRarelyChangedCB;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dRarelyChangedTextureCB;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> m_textureSamplerState;

			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

			std::wstring m_rootString;

		};

	} // demo
} // xtest