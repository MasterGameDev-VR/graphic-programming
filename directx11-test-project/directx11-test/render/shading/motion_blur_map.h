#pragma once
#include <render/renderable.h>
#include <render/renderable_in_motion.h>
#include <application/directx_app.h>
#include <camera/spherical_camera.h>

namespace xtest {
	namespace render {
		namespace shading {
			class MotionBlurMap
			{
			public:
				struct PerObjectMotionBlurMapData
				{
					DirectX::XMFLOAT4X4 WVP_currentFrame;
					DirectX::XMFLOAT4X4 WVP_previousFrame;
				};

				explicit MotionBlurMap();
				void SetWidthHeight(uint32 width, uint32 height);

				void Init(uint32 width, uint32 height);

				void SetViewAndProjectionMatrices(const xtest::camera::SphericalCamera& camera);
				PerObjectMotionBlurMapData ToPerObjectMotionBlurMapData(const render::Renderable& renderableInMotion, const std::string& meshName, const xtest::camera::SphericalCamera& cameraRef, const DirectX::XMFLOAT4X4& prevoius);
				ID3D11RenderTargetView* AsMotionBlurView();
				ID3D11ShaderResourceView* AsShaderView();

				

				D3D11_VIEWPORT Viewport() const;

			private:

				uint32 m_width;
				uint32 m_height;
				uint32 m_resolution;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_motionBlurShaderView;
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_motionBlurView;
				D3D11_VIEWPORT m_viewport;

				//la mappa deve essere realizzata usando come punto di vista quello della camera
				DirectX::XMFLOAT3 m_cameraDir;
				DirectX::XMFLOAT3 m_up;

				//questi membri privati servono alla motionBlurMap
				//sono unici e collegati alla camera, al punto e alla direzione  da cui osserva
				//questi vanno cambiati solo se si muove la camera
				DirectX::XMFLOAT4X4 m_V;
				DirectX::XMFLOAT4X4 m_P;
				bool m_isDirty;

				//ho fatto diventare i descrittori e il COM pointer della texture 2D membri di classe
				D3D11_TEXTURE2D_DESC textureDesc;
				Microsoft::WRL::ComPtr<ID3D11Texture2D> motionVector;  
				D3D11_RENDER_TARGET_VIEW_DESC motionBlurViewDesc;
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewMotionDesc;



			};
		}
	}
}