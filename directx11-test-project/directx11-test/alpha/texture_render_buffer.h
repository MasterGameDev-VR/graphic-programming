#pragma once
#include <render/render_resource.h>

namespace alpha {
	class TextureRenderBuffer {

	public:
		TextureRenderBuffer();

		void Init(uint32 width, uint32 height);

		void Resize(uint32 width, uint32 height);

		ID3D11ShaderResourceView* AsShaderView();
		ID3D11RenderTargetView* AsRenderTargetView();

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	};
}