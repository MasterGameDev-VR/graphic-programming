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


	class PCFSampler : public Sampler
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
			samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.BorderColor[0] = 1.f;
			samplerDesc.BorderColor[1] = 1.f;
			samplerDesc.BorderColor[2] = 1.f;
			samplerDesc.BorderColor[3] = 1.f;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};

	class NormalDepthSampler : public Sampler
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
			samplerDesc.BorderColor[3] = 1e5f;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_EQUAL;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};

	class RandomVecSampler : public Sampler
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
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_EQUAL;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};

	class SSAOMapSampler : public Sampler
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
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_EQUAL;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};

	class SSAOBlurSampler : public Sampler
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
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_EQUAL;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateSamplerState(&samplerDesc, &m_d3dTextureSampler));
		}
	};

} //shading
} //render
} //xtest

