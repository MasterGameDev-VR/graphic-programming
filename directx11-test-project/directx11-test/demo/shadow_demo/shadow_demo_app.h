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


namespace xtest {
namespace demo {

	/*
		Use F1 switch on/off the shadow casting
		Use ALT+Enter to switch full screen on/off
		Use F key to reframe the camera to the origin
		Use the middle mouse button/wheel button and drag to rotate the light direction
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
			float _explicit_pad_[2];
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
		PerObjectData ToPerObjectData(const render::Renderable& renderable, const std::string& meshName);
		PerObjectShadowMapData ToPerObjectShadowMapData(const render::Renderable& renderable, const std::string& meshName);


		DirectionalLight m_dirKeyLight;
		DirectionalLight m_dirFillLight;
		RarelyChangedData m_rarelyChangedData;
		bool m_isRarelyChangedDataDirty;

		camera::SphericalCamera m_camera;
		std::vector<render::Renderable> m_objects;
		render::shading::RenderPass m_shadowPass;
		render::shading::RenderPass m_renderPass;
		render::shading::ShadowMap m_shadowMap;
		scene::BoundingSphere m_sceneBoundingSphere;
	};

} // demo
} // xtest


