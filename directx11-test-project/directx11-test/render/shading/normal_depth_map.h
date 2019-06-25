#pragma once

#include <render/render_resource.h>
#include <common/int_types.h>
#include <d3d11.h>
#include <wrl/implements.h>


namespace xtest {
	namespace render {
		namespace shading {

			// class representing a Normal Depth map
			class NormalDepthMap
			{
			public:

				explicit NormalDepthMap(uint32 width, uint32 height);

				void Init();
				ID3D11ShaderResourceView* AsShaderView();
				ID3D11RenderTargetView* AsRenderTargetView();
				D3D11_VIEWPORT Viewport() const;
				void ResetTargetView(uint32 n_width, uint32 n_height);
				uint32 Width() const;
				uint32 Height() const;

			private:
				uint32 m_width;
				uint32 m_height;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderView;
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

				D3D11_VIEWPORT m_viewport;
			};

		} //shading
	} //render
} //xtest

