#pragma once
#include <render/render_resource.h>
#include <render/shading/ssao_map.h>

namespace xtest {
	namespace render {
		namespace shading {

			// class representing a shadow map
			class BlurMap
			{
			public:
			
				explicit BlurMap(
					uint32 width,
					uint32 height
				);

				void Init();

				ID3D11ShaderResourceView* AsShaderView();
				ID3D11RenderTargetView* AsRenderTargetView();
				D3D11_VIEWPORT Viewport() const;
				void SetViewport(D3D11_VIEWPORT& view);
				void ResetTargetView(uint32 n_width, uint32 n_height);

				uint32 Width() const;
				uint32 Height() const;
				static float RandomFloat(float, float);
				void BuildFrustumFarCorners(float aspect, float fovy, float farZ);
				DirectX::XMFLOAT4* GetFrustumFarCorner();
				DirectX::XMFLOAT4* GetKernelVectors();

				void Bind();
				void Draw();

			private:
				SSAOData m_vs_data;
				Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dVertexBuffer;
				Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dIndexBuffer;

				uint32 m_height;
				uint32 m_width;

				DirectX::XMFLOAT4 m_frustumFarCorner[4];

				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderView;
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
				D3D11_VIEWPORT m_viewport;
			};
		} //shading
	} //render
} //xtest
