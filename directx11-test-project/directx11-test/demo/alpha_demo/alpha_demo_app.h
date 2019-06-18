#pragma once
#pragma once


#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>
#include <render/renderable.h>
#include <render/shading/render_pass.h>
#include <scene/bounding_sphere.h>
#include <render/shading/shadow_map.h>
#include <alpha/texture_render_buffer.h>
#include <alpha/texture_renderable.h>
#include <alpha/alpha_types.h>

namespace xtest {
	namespace demo {

		/*
			Use F1 switch on/off the shadow casting
			Use ALT+Enter to switch full screen on/off
			Use F key to reframe the camera to the origin
			Use the middle mouse button/wheel button and drag to rotate the light direction
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to move forward
		*/

		class AlphaDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
		{
		public:

			typedef std::pair<const render::Renderable*, std::string> GlowObjectKey;

			struct DirectionalLight
			{
				DirectX::XMFLOAT4 ambient;
				DirectX::XMFLOAT4 diffuse;
				DirectX::XMFLOAT4 specular;
				DirectX::XMFLOAT3 dirW;
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
				Material material;
			};

			struct PerObjectGlowData
			{
				DirectX::XMFLOAT4X4 WVP;
				DirectX::XMFLOAT4X4 TexcoordMatrix;
				int32 useGlow;
				float _explicit_pad_[3];
			};

			struct PerFrameBlurData
			{
				float resolution;
				float _explicit_pad_[3];
			};

			struct PerObjectShadowMapData
			{
				DirectX::XMFLOAT4X4 WVP_lightSpace;
			};

			static const int k_pointLightCount = 4;
			static const int k_dirLightCount = 2;
			struct PerFrameData
			{
				DirectionalLight dirLights[k_dirLightCount];
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
			};

			struct RarelyChangedData
			{
				int32 useShadowMap;
				int32 useGlowMap;
				float shadowMapResolution;
				float _explicit_pad_;
			};



			AlphaDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~AlphaDemoApp();

			AlphaDemoApp(AlphaDemoApp&&) = delete;
			AlphaDemoApp(const AlphaDemoApp&) = delete;
			AlphaDemoApp& operator=(AlphaDemoApp&&) = delete;
			AlphaDemoApp& operator=(const AlphaDemoApp&) = delete;


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
			void InitRenderToTexture();
			void InitGlowMap();
			void CreateDownDepthStencilBuffer();
			PerObjectData ToPerObjectData(const render::Renderable& renderable, const std::string& meshName);
			PerObjectGlowData ToPerObjectGlowData(const render::Renderable& renderable, const std::string& meshName);
			PerObjectShadowMapData ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName);


			DirectionalLight m_dirKeyLight;
			DirectionalLight m_dirFillLight;
			RarelyChangedData m_rarelyChangedData;
			bool m_isRarelyChangedDataDirty;

			D3D11_VIEWPORT m_downViewport;

			camera::SphericalCamera m_camera;
			std::vector<render::Renderable> m_objects;
			std::map<GlowObjectKey, alpha::GlowObject> m_glowObjectsMap;
			alpha::TextureRenderable m_textureRenderable;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBufferDownsample;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilViewDownsample;
			render::shading::RenderPass m_shadowPass;
			render::shading::RenderPass m_renderPass;
			render::shading::RenderPass m_glowPass;
			render::shading::RenderPass m_downPass;
			render::shading::RenderPass m_upPass;
			render::shading::RenderPass m_horizontalBlurPass;
			render::shading::RenderPass m_verticalBlurPass;
			render::shading::RenderPass m_PostPass;
			render::shading::ShadowMap m_shadowMap;
			alpha::TextureRenderBuffer m_sceneTexture;
			alpha::TextureRenderBuffer m_downsampledGlowTexture; 
			alpha::TextureRenderBuffer m_upsampledGlowTexture;
			alpha::TextureRenderBuffer m_glowmap;
			alpha::TextureRenderBuffer m_horizontalBlurTexture;
			alpha::TextureRenderBuffer m_verticalBlurTexture;
			scene::BoundingSphere m_sceneBoundingSphere;
		};

	} // demo
} // xtest


