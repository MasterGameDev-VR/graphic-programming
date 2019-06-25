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
#include <render/shading/ssao_map.h>
#include <render/shading/blur_map.h>
#include <render/shading/normal_depth_map.h>
#include <render/shading/random_vec_map.h>


namespace xtest {
	namespace demo {

		/*
			Use F1 switch on/off the shadow casting
			Use ALT+Enter to switch full screen on/off
			Use F key to reframe the camera to the origin
			Use the middle mouse button/wheel button and drag to rotate the light direction
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to move forward
		*/

		class SSAODemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
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
				DirectX::XMFLOAT4X4 WVPT;
				DirectX::XMFLOAT4X4 TexcoordMatrix;
				DirectX::XMFLOAT4X4 WVPT_shadowMap;
				Material material;
			};

			struct PerFrameDataNormalDepth
			{
				DirectX::XMFLOAT4X4 worldView;
				DirectX::XMFLOAT4X4 worldInvTransposeView;
				DirectX::XMFLOAT4X4 worldViewProj;
				DirectX::XMFLOAT4X4 texTransform;
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
				float shadowMapResolution;
				int32 useSSAOMap;
				float SSOAMultiplier;
			};

			struct PerObjectCBAmbientOcclusion
			{
				DirectX::XMFLOAT4X4 viewToTexSpace; // Proj*Tex
				DirectX::XMFLOAT4 offsetVectors[xtest::render::shading::SSAOData::SAMPLE_COUNT];
				DirectX::XMFLOAT4 frustumCorners[4];

				float occlusionRadius;
				float occlusionFadeStart;
				float occlusionFadeEnd;
				float surfaceEpsilon;
			};

			struct BlurCBuffer // new struct
			{
				float texelWitdth;
				float texelHeight;
				uint32 horizontalBlur;
				float pad;
			};



			SSAODemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~SSAODemoApp();

			SSAODemoApp(SSAODemoApp&&) = delete;
			SSAODemoApp(const SSAODemoApp&) = delete;
			SSAODemoApp& operator=(SSAODemoApp&&) = delete;
			SSAODemoApp& operator=(const SSAODemoApp&) = delete;


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
			void InitTestRenderables();
			void InitLights();
			PerObjectData ToPerObjectData(const render::Renderable& renderable, const std::string& meshName);
			PerObjectCBAmbientOcclusion ToPerObjectAmbientOcclusion();
			PerFrameDataNormalDepth ToPerFrameData(const render::Renderable& renderable);

			PerObjectShadowMapData ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName);

			BlurCBuffer ToPerFrameBlur(bool);

			DirectionalLight m_dirKeyLight;
			DirectionalLight m_dirFillLight;
			RarelyChangedData m_rarelyChangedData;
			bool m_isRarelyChangedDataDirty;

			camera::SphericalCamera m_camera;
			std::vector<render::Renderable> m_objects;

			render::shading::RenderPass m_shadowPass;
			render::shading::RenderPass m_normalDepthPass;
			render::shading::RenderPass m_SSAOPass;
			render::shading::RenderPass m_SSAOBlurPass;
			render::shading::RenderPass m_SSAOBlurHPass;
			render::shading::RenderPass m_renderPass;

			render::shading::ShadowMap m_shadowMap;
			render::shading::SSAOMap m_SSAOMap;
			render::shading::BlurMap m_BlurHMap;
			render::shading::BlurMap m_BlurMap;
			render::shading::NormalDepthMap m_normalDepthMap;
			render::shading::RandomVectorMap m_randomVecMap;
			scene::BoundingSphere m_sceneBoundingSphere;
		};

	} // demo
} // xtest
