#pragma once

#include <render/shading/sampler.h>
#include <service/locator.h>

namespace alpha
{
	class BlurSampler : public xtest::render::shading::Sampler
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
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.BorderColor[0] = 0.f;
			samplerDesc.BorderColor[1] = 0.f;
			samplerDesc.BorderColor[2] = 0.f;
			samplerDesc.BorderColor[3] = 0.f;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

			XTEST_D3D_CHECK(xtest::service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};
}