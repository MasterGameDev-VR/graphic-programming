#pragma once
//#include <application/windows_app.h>
//#include <application/directx_settings.h>
//#include <file/resource_loader.h>


namespace xtest
{
	namespace demo
	{
		class ShadowUtilities 
		{
		public:
			ShadowUtilities();
			virtual ~ShadowUtilities() {};

			ShadowUtilities(ShadowUtilities&&) = delete;
			ShadowUtilities(const ShadowUtilities&) = delete;
			ShadowUtilities& operator=(ShadowUtilities&&) = delete;
			ShadowUtilities& operator=(const ShadowUtilities&) = delete;


			//Microsoft::WRL::ComPtr <>
			Microsoft::WRL::ComPtr < ID3D11Texture2D> m_shadowDepthBuffer;
			Microsoft::WRL::ComPtr < ID3D11DepthStencilView> m_shadowDepthBufferView;
			Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> m_shadowShaderResourceView;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_shadowDepthStencilState;
			D3D11_VIEWPORT m_shadowsViewPort;
			unsigned m_shadowsTextureResolution;

			void Init();
			void CreateShadowDepthStencilBufferAndViews();

			//const ID3D11ShaderResourceView* GetShadowShaderResourceView();

			void SetViewPort();

			D3D11_TEXTURE2D_DESC shadowDepthBufferDesc;
			D3D11_DEPTH_STENCIL_DESC shadowDepthStencilDesc;
			D3D11_DEPTH_STENCIL_VIEW_DESC shadowDepthBufferViewDesc;
			D3D11_SHADER_RESOURCE_VIEW_DESC shadowShaderResViewDesc;


		protected:
			Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;

		};

	}
}