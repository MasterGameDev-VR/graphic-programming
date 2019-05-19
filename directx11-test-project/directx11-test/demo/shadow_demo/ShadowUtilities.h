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
			ShadowUtilities(ID3D11Device*);
			ShadowUtilities();
			virtual ~ShadowUtilities() {};

			ShadowUtilities(ShadowUtilities&&) = delete;
			ShadowUtilities(const ShadowUtilities&) = delete;
			ShadowUtilities& operator=(ShadowUtilities&&) = delete;
			ShadowUtilities& operator=(const ShadowUtilities&) = delete;


		
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_shadowDepthBuffer;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDepthBufferView;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowShaderResourceView;
			D3D11_VIEWPORT m_shadowsViewPort;
			uint32 m_shadowsTextureResolution;

			void Init();
			void CreateShadowDepthStencilBufferAndViews();

			void SetViewPort();

		protected:
			Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;

		};

	}
}