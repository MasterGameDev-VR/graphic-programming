#pragma once


#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>
#include <render/renderable.h>
#include <render/shading/render_pass.h>


namespace xtest {
	namespace demo {

		/*
			Use F1 and F2 to switch on/off the lights
			Use F3 to turn bump mapping on/off
			Use F4 to turn bump mapping on/off
			Use Spacebar to pause lights motion
			Usa ALT+Enter to switch full screen on/off
			Use F key to reframe the camera to the origin
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to move forward
		*/

		class ShadowDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
		{
		public:


			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 dirW;
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

			struct Material
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
			};

			struct PerObjectData
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_inverseTraspose;
				DirectX::XMFLOAT4X4 WVP;
				DirectX::XMFLOAT4X4 TexcoordMatrix;
				DirectX::XMFLOAT4X4 WVPT_shadowMap;
				DirectX::XMFLOAT4X4 WVPT_projector;
				Material material;
			};

			static const int k_pointLightCount = 4;
			static const int k_dirLightCount = 2;
			struct PerFrameData
			{
				DirectionalLight dirLights[k_dirLightCount];
				PointLight pointLights[k_pointLightCount];
				SpotLight spotLight;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
			};

			struct RarelyChangedData
			{
				int32 useDirLight;
				int32 usePointLight;
				int32 useSpotLight;
				int32 useBumpMap;
			};


			ShadowDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~ShadowDemoApp();

			ShadowDemoApp(ShadowDemoApp&&) = delete;
			ShadowDemoApp(const ShadowDemoApp&) = delete;
			ShadowDemoApp& operator=(ShadowDemoApp&&) = delete;
			ShadowDemoApp& operator=(const ShadowDemoApp&) = delete;


			virtual void Init() override;
			virtual void OnResized() override;
			virtual void UpdateScene(float deltaSeconds) override;
			virtual void RenderScene() override;

			virtual void OnWheelScroll(input::ScrollStatus scroll) override;
			virtual void OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos) override;
			virtual void OnKeyStatusChange(input::Key key, const input::KeyStatus& status) override;

		private:

			void InitRenderTechnique();
			void InitRenderables();
			void InitLights();
			void InitShadowMap();
			PerObjectData ToPerObjectData(const render::Renderable& renderable, const std::string& meshName) const;
			PerObjectData ToPerObjectData_ShadowMap(const render::Renderable& renderable, const std::string& meshName) const;
			PerObjectData ToPerObjectData_Projector(const render::Renderable& renderable, const std::string& meshName) const;


			// Shadow mapping
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_shadowMap;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowMapDB;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowMapPS;
			D3D11_VIEWPORT m_shadowMapViewport;
			DirectX::BoundingSphere m_bSphere;
			DirectX::XMMATRIX m_LightViewMatrix;
			DirectX::XMMATRIX m_LightProjectionMatrix;
			DirectX::XMMATRIX m_T;
			unsigned int m_resolution;

			// Projector
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_projector;
			Microsoft::WRL::ComPtr<ID3D11Resource> m_projectorTexture;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_projectorDB;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_projectorPS;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_projectorTexturePS;
			D3D11_VIEWPORT m_projectorViewport;
			DirectX::BoundingSphere m_projectorBSphere;
			DirectX::XMMATRIX m_SpotLightViewMatrix;
			DirectX::XMMATRIX m_SpotLightProjectionMatrix;
			DirectX::XMMATRIX m_projectorT;
			unsigned int m_projectorResolution;


			DirectionalLight m_dirKeyLight;
			DirectionalLight m_dirFillLight;
			SpotLight m_spotLight;
			PointLight m_pointLight;
			RarelyChangedData m_lightingControls;
			bool m_isLightingControlsDirty;
			bool m_stopLights;

			camera::SphericalCamera m_camera;
			std::vector<render::Renderable> m_objects;
			render::shading::RenderPass m_renderPass;
			render::shading::RenderPass m_firstRenderPass;
			render::shading::RenderPass m_firstRenderPassProjector;
		};

	} // demo
} // xtest


