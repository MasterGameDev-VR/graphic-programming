#pragma once

#include <render/shading/sampler.h>
#include <service/locator.h>


namespace xtest {
namespace render {
namespace shading {

	/************************************************************************/
	/* house of all the texture sampler implementations used                */
	/************************************************************************/


	class AnisotropicSampler : public Sampler
	{
	public:
		virtual void Init() override
		{
			// already initialized
			if (m_d3dTextureSampler)
			{
				return;
			}

			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = 16;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};


	

} //shading
} //render
} //xtest

