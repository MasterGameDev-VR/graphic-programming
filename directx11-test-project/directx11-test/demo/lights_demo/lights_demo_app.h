#pragma once
#include <application/directx_app.h>
#include <input/keyboard.h>
#include <camera/spherical_camera.h>
#include <mesh/mesh_generator.h>

using namespace xtest::mesh;
namespace xtest
{
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
			/*
			struct VertexIn
			{
				DirectX::XMFLOAT3 pos;
				float _explicit_pad_1_;
				DirectX::XMFLOAT3 normalL;
				float _explicit_pad_2_;
			};
			*/

			struct PerObjectCB
			{
				DirectX::XMFLOAT4X4 W;
				DirectX::XMFLOAT4X4 W_Inv_Transp;
				DirectX::XMFLOAT4X4 WVP;
				Material material;
			};
			struct PerFrameCB
			{
				DirectionalLight dirLight;
				PointLight pointLight;
				SpotLight spotLight;
				DirectX::XMFLOAT3 eyePosW;
				float _explicit_pad_;
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
			void InitLights();
			void InitMaterials();
			void InitShaders();
			void InitBuffers();
			void InitRasterizerState();


			DirectX::XMFLOAT4X4 m_worldMatrixPlane;
			DirectX::XMFLOAT4X4 m_worldMatrixTorus;
			DirectX::XMFLOAT4X4 m_worldMatrixBox;
			DirectX::XMFLOAT4X4 m_worldMatrixSphere;

			DirectX::XMFLOAT4X4 m_viewMatrix;
			DirectX::XMFLOAT4X4 m_projectionMatrix;

			camera::SphericalCamera m_camera;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferPlane;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferPlane;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferTorus;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferTorus;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferBox;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferBox;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferSphere;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferSphere;
			//-----------------------------------------------------
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBufferPlane;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBufferTorus;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBufferSphere;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vsConstantBufferBox;

			Microsoft::WRL::ComPtr<ID3D11Buffer> m_psConstantBufferLights;

			//----------OBJECT MATERIALS 
			//DirectX::XMFLOAT4 color_plane = { 0.0f,0.502f,0.0f,1.0f }; //green
			DirectX::XMFLOAT4 mat_plane_rgba_ambient;
			DirectX::XMFLOAT4 mat_plane_rgba_diffuse;
			DirectX::XMFLOAT4 mat_plane_rgba_specular;

			DirectX::XMFLOAT4 mat_torus_rgba_ambient;
			DirectX::XMFLOAT4 mat_torus_rgba_diffuse;
			DirectX::XMFLOAT4 mat_torus_rgba_specular;

			DirectX::XMFLOAT4 mat_sphere_rgba_ambient;
			DirectX::XMFLOAT4 mat_sphere_rgba_diffuse;
			DirectX::XMFLOAT4 mat_sphere_rgba_specular;

			DirectX::XMFLOAT4 mat_box_rgba_ambient;
			DirectX::XMFLOAT4 mat_box_rgba_diffuse;
			DirectX::XMFLOAT4 mat_box_rgba_specular;

			Material mat_plane;
			Material mat_torus;

			Material mat_sphere;  //orange-red
			Material mat_box; //tomato

			//----LIGHTS   YEAH
			DirectX::XMFLOAT4 color_dirLight_ambient;
			DirectX::XMFLOAT4 color_dirLight_diffuse;
			DirectX::XMFLOAT4 color_dirLight_specular;
			DirectX::XMFLOAT4 color_pointLight_ambient;
			DirectX::XMFLOAT4 color_pointLigh_diffuse;
			DirectX::XMFLOAT4 color_pointLight_specular;
			DirectX::XMFLOAT4 color_spotLight_ambient;
			DirectX::XMFLOAT4 color_spotLight_diffuse;
			DirectX::XMFLOAT4 color_spotLight_specular;

			DirectX::XMFLOAT3 dirSpotLight;
			DirectX::XMFLOAT3 eyePosW;

			
			DirectionalLight myDirectionalLight; // colors ---- direction -------- padding float
			PointLight myPointLight; // colors --------position ------- range ------- attenuation ---- padding float
			SpotLight mySpotLight;// colors --------position ------- range -------direction ----------- spot dimension (angle, radians)---------  attenuation ---- padding float
			float _explicit_pad_{ 0.0f };
			//PerFrameCB myPerFrameCB = { myDirectionalLight ,myPointLight,mySpotLight,0.0f };

			Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
		};
	}
}
