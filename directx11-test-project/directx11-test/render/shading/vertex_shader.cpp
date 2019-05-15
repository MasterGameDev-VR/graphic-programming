#include "stdafx.h"
#include "vertex_shader.h"
#include <service/locator.h>

using namespace xtest::render::shading;



void VertexShader::Init()
{
	XTEST_ASSERT(m_vertexInput, L"no vertex input set");

	// already initialized
	if (m_d3dVertexShader)
	{
		return;
	}

	Shader::Init();
	m_vertexInput->Init();
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateVertexShader(m_byteCode->Data(), m_byteCode->ByteSize(), nullptr, &m_d3dVertexShader));
}


void VertexShader::Bind()
{
	XTEST_ASSERT(m_vertexInput, L"no vertex input set");
	XTEST_ASSERT(m_d3dVertexShader, L"unitialized vertex shader");
	
	m_vertexInput->Bind();
	service::Locator::GetD3DContext()->VSSetShader(m_d3dVertexShader.Get(), nullptr, 0);
		
	for (CBufferMap::reference frequencyPairWithCBuffer : m_cbufferMapByFrequency)
	{
		frequencyPairWithCBuffer.second->SetBindingTarget(ShaderTarget::vertex_shader);
		frequencyPairWithCBuffer.second->SetFrequency(frequencyPairWithCBuffer.first);
		frequencyPairWithCBuffer.second->Bind();
	}

	for (SamplerMap::reference usagePairWithSampler : m_samplerMapByUsage)
	{
		usagePairWithSampler.second->SetBindingTarget(ShaderTarget::vertex_shader);
		usagePairWithSampler.second->SetUsage(usagePairWithSampler.first);
		usagePairWithSampler.second->Bind();
	}

}


void VertexShader::BindTexture(TextureUsage usage, const ID3D11ShaderResourceView* texture)
{
	XTEST_ASSERT(usage != TextureUsage::uknown);
	service::Locator::GetD3DContext()->VSSetShaderResources(static_cast<int>(usage), 1, const_cast<ID3D11ShaderResourceView**>(&texture));
}



void VertexShader::SetVertexInput(std::shared_ptr<VertexInput> vertexInput)
{
	XTEST_ASSERT(!m_vertexInput, L"vertex input already set");
	m_vertexInput = vertexInput;
	m_vertexInput->SetVertexShaderByteCode(m_byteCode);
}



