#pragma once

#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>
#include <render/renderable.h>
#include <render/shading/render_pass.h>
#include<demo/shadow_demo/ShadowUtilities.h>

namespace xtest
{
	namespace demo
	{

		class ShadowDemoApp : public application::DirectxApp, public ShadowUtilities, public input::MouseListener, public input::KeyboardListener
		{
		public:
			//structs which define lights, material, per object data, etc
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
				Material material;
			};

			struct BoundingSphere {
				DirectX::XMFLOAT4 bSpherePositionLS;
				float radius;
				float view_Left;
				float view_Right;
				float view_Bottom;
				float view_Top;
				float view_NearZ;
				float view_FarZ;
				float _explicit_pad_;
			};

			//number of point lights and directional lights
			static const int k_pointLightCount = 4;
			//a directional light is used to enlighten the shadowed areas of the scene
			static const int k_dirLightCount = 2;
			//
			static const int k_lightsThatCastShadows = 1;
			struct PerFrameData
			{
				DirectionalLight dirLights[k_dirLightCount];
				PointLight pointLights[k_pointLightCount];
				DirectX::XMFLOAT4X4 LightViewMatrices[k_lightsThatCastShadows];
				DirectX::XMFLOAT4X4 ProjectionMatrices[k_lightsThatCastShadows];
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
			};
			
			/*
			struct PerObjectDataForShadows 
			{

			};
			struct PerFrameDataForShadows 
			{
				DirectX::XMFLOAT4X4 LightViewMatrices[k_lightsThatCastShadows];
			};
			*/

			struct RarelyChangedData
			{
				int32 useDirLight;
				int32 usePointLight;
				int32 useBumpMap;
				int32 _explicit_pad_;
			};

			//constructor and destructor
			ShadowDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~ShadowDemoApp();

			//do not automatically generate move and copy constructors and move and copy assignment operators
			ShadowDemoApp(ShadowDemoApp&&) = delete;
			ShadowDemoApp(const ShadowDemoApp&) = delete;
			ShadowDemoApp& operator=(ShadowDemoApp&&) = delete;
			ShadowDemoApp& operator=(const ShadowDemoApp&) = delete;

			//override from directx_app
			virtual void Init() override; // it calls the methods WindowsApp::Init(), InitDirectX() (defined in DirectXApp), ProvideD3DDevice, ProvideD3DContext, ProvideResourceLocator
			virtual void OnResized() override;  //this method calls ResizeBuffers

			//override from directx_app where they are defined as pure virtual functions
			virtual void UpdateScene(float deltaSeconds) override;
			virtual void RenderScene() override;

			virtual void OnWheelScroll(input::ScrollStatus scroll) override;
			virtual void OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos) override;
			virtual void OnKeyStatusChange(input::Key key, const input::KeyStatus& status) override;

			void RenderShadows();

		private:

			void InitShadowRenderTechnique();
			//void UpdateShadowRenderPass();

			void InitRenderTechnique();
			void InitRenderables();
			void InitLights();
			PerObjectData ToPerObjectData(const render::Renderable& renderable, const std::string& meshName) const;

			DirectionalLight m_dirKeyLight;
			DirectionalLight m_dirFillLight;
			BoundingSphere m_boundingSphere;
			PointLight m_pointLight;
			RarelyChangedData m_lightingControls;
			bool m_isLightingControlsDirty;
			bool m_stopLights;

			camera::SphericalCamera m_camera;
			std::vector<render::Renderable> m_objects;
			render::shading::RenderPass m_renderPass;
			render::shading::RenderPass m_shadowRenderPass;

			XMFLOAT4X4 LVMtemp[k_lightsThatCastShadows];
			XMFLOAT4X4 PrMatrtemp[k_lightsThatCastShadows];

		};
	}
}