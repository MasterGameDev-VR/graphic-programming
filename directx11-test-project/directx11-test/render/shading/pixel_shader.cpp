#include "stdafx.h"
#include "pixel_shader.h"
#include <service/locator.h>


using namespace xtest::render::shading;


void PixelShader::Bind()
{
	XTEST_ASSERT(m_d3dPixelShader, L"uninitialized pixel shader");

	service::Locator::GetD3DContext()->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);

	for (CBufferMap::reference frequencyPairWithCBuffer : m_cbufferMapByFrequency)
	{
		frequencyPairWithCBuffer.second->SetBindingTarget(ShaderTarget::pixel_shader);
		frequencyPairWithCBuffer.second->SetFrequency(frequencyPairWithCBuffer.first);
		frequencyPairWithCBuffer.second->Bind();
	}

	for (SamplerMap::reference usagePairWithSampler : m_samplerMapByUsage)
	{
		usagePairWithSampler.second->SetBindingTarget(ShaderTarget::pixel_shader);
		usagePairWithSampler.second->SetUsage(usagePairWithSampler.first);
		usagePairWithSampler.second->Bind();
	}
}


void PixelShader::BindTexture(TextureUsage usage, const ID3D11ShaderResourceView* texture)
{
	XTEST_ASSERT(usage != TextureUsage::uknown);
	service::Locator::GetD3DContext()->PSSetShaderResources(static_cast<int>(usage), 1, const_cast<ID3D11ShaderResourceView**>(&texture));
}


void PixelShader::Init()
{
	// already initialized
	if (m_d3dPixelShader)
	{
		return;
	}

	Shader::Init();
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreatePixelShader(m_byteCode->Data(), m_byteCode->ByteSize(), nullptr, &m_d3dPixelShader));
}
