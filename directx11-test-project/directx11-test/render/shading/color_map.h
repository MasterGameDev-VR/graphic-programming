#pragma once


namespace xtest {
	namespace render {
		namespace shading {
			class ColorMap
			{
			public:
				explicit ColorMap();
				void SetWidthHeight(uint32 width, uint32 height);

				void Init();


				ID3D11RenderTargetView* AsColorRenderTargetView();
				ID3D11ShaderResourceView* AsColorShaderView();

				D3D11_VIEWPORT Viewport() const;

			private:

				uint32 m_width;
				uint32 m_height;
				uint32 m_resolution;
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_colorShaderView;
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_colorRenderView;
				D3D11_VIEWPORT m_viewport;

			};
		}
	}
}