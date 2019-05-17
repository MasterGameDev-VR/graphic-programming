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
			ShadowUtilities();
			virtual ~ShadowUtilities() {};

			ShadowUtilities(ShadowUtilities&&) = delete;
			ShadowUtilities(const ShadowUtilities&) = delete;
			ShadowUtilities& operator=(ShadowUtilities&&) = delete;
			ShadowUtilities& operator=(const ShadowUtilities&) = delete;


		public:
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_shadowDepthBuffer;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDepthBufferView;

			void CreateShadowDepthStencilBuffer();

		};

	}
}