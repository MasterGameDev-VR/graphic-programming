#pragma once
#include <application/directx_app.h>
#include <input/mouse.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>
#include <mesh/mesh_format.h>
#include <render/renderable.h>
#include <render/shading/render_pass.h>
#include <render/shading/shadow_map.h>
#include <render/shading/motion_blur_map.h>
#include <render/shading/color_map.h>
#include "Quad.h"


/*
Il motion blur è una tecnica di post - processing che si realizza su oggetti in movimento e permette a questi ultimi
di lasciare una scia semi - invisibile durante il loro movimento
Per implementarlo è necessario avere una texture2D usata come buffer, avente width e height uguali a quelli della
scena da renderizzare, e nella quale vengono immagazzinate le DIREZIONI DI MOVIMENTO DI CIASCUNO dei pixel che compongono un oggett
questa texture deve essere riempita con una passata di rendering
*/
/* dovremmo creare 
  - una classe  nel namespace xtest/demo
 che eredita da DirectxApp, MouseListener e Keyboard Listener
 - in questa classe inseriamo un oggetto di tipo RenderPass che chiamiamo MotionBlurPass
 - poi creiamo una classe MotionBlurMap usando la gerarchia suggerita dal professore
 - nella classe MotionBlurMap dovremmo inserire una Texture2D nella quale memorizziamo per ogni pixel
 la distanza tra la posizione in cui si trova nel frame corrente e quella in cui si trovava nel frame precedente
 - la passata di rendering deve inserire i dati in questa texture
 - serviranno una struct PerObjectMotionBlurData e un metodo ToPerObjectMotionBlurData: in questa struct ci mettiamo
  le matrici ottenute dai prodotti WVP relative a due frame (quello attuale e quello precedente)
- 

*/

namespace xtest {
	namespace demo {

		/*
			Use F1 switch on/off the shadow casting
			Use ALT+Enter to switch full screen on/off
			Use F key to reframe the camera to the origin
			Use the middle mouse button/wheel button and drag to rotate the light direction
			Use right mouse button to pan the view, left mouse button to rotate and mouse wheel to move forward
		*/

		class MotionBlurDemoApp : public application::DirectxApp, public input::MouseListener, public input::KeyboardListener
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

			struct PerObjectCombineData {
				DirectX::XMFLOAT4X4 WVP;
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
				int32 useMotionBlurMap;
				float _explicit_pad_[1];
			};



			MotionBlurDemoApp(HINSTANCE instance, const application::WindowSettings& windowSettings, const application::DirectxSettings& directxSettings, uint32 fps = 60);
			~MotionBlurDemoApp();

			MotionBlurDemoApp(MotionBlurDemoApp&&) = delete;
			MotionBlurDemoApp(const MotionBlurDemoApp&) = delete;
			MotionBlurDemoApp& operator=(MotionBlurDemoApp&&) = delete;
			MotionBlurDemoApp& operator=(const MotionBlurDemoApp&) = delete;


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
			PerObjectCombineData ToPerObjectCombineData(const render::Renderable& renderable, const std::string& meshName);

			DirectionalLight m_dirKeyLight;
			DirectionalLight m_dirFillLight;
			RarelyChangedData m_rarelyChangedData;
			bool m_isRarelyChangedDataDirty;

			camera::SphericalCamera m_camera;
			std::vector<render::Renderable> m_objects;
			std::vector<DirectX::XMFLOAT4X4> previous_Transforms;
			render::shading::RenderPass m_shadowPass;
			render::shading::RenderPass m_renderPass;
			render::shading::RenderPass m_motionBlurPass;
			render::shading::RenderPass m_combinePass;
			render::shading::ShadowMap m_shadowMap;
			render::shading::MotionBlurMap m_motionBlurMap;

			render::shading::ColorMap m_colorRenderMap;
			Quad m_quad;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_quadVertexBuffer;

			scene::BoundingSphere m_sceneBoundingSphere;
		};

	} // demo
} // xtest


