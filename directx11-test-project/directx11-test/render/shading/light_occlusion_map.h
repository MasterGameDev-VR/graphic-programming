#pragma once

#include <render/render_resource.h>
#include <scene/bounding_sphere.h>
#include <render/renderable.h>

namespace xtest {
namespace render {
namespace shading {

	// class representing a occlusion map for light scattering
	class LightOcclusionMap
	{
	public:

		explicit LightOcclusionMap(uint32 resolution);

		void Init();

		void SetTargetBoundingSphere(const scene::BoundingSphere& boundingSphere);
		void SetLight(const DirectX::XMFLOAT3& dirLight, const DirectX::XMFLOAT3& up = { 0.f,1.f,0.f });

		ID3D11RenderTargetView* AsRenderTargetView();
		ID3D11ShaderResourceView* AsShaderView();

		D3D11_VIEWPORT Viewport() const;
		ID3D11DepthStencilView* DepthBufferView();
		uint32 Resolution() const;
		render::Renderable LightPlaceHolder();

		DirectX::XMMATRIX TMatrix() const;

	private:

		void CalcMatrices();

		uint32 m_resolution;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthBufferView;
		D3D11_VIEWPORT m_viewport;
		scene::BoundingSphere m_bSphere;
		render::Renderable m_lightPlaceHolder;
		DirectX::XMFLOAT3 m_lightDir;
		bool m_isDirty;
	};

} //shading
} //render
} //xtest
