#include "stdafx.h"
#include "shader.h"

using namespace xtest::render::shading;


Shader::Shader(const xtest::file::BinaryFile* binaryFile)
	: m_cbufferMapByFrequency()
	, m_samplerMapByUsage()
	, m_byteCode(binaryFile)
{}


void Shader::AddConstantBuffer(CBufferFrequency frequency, std::unique_ptr<CBufferBase> cbuffer)
{
	XTEST_ASSERT(m_cbufferMapByFrequency.find(frequency) == m_cbufferMapByFrequency.end(), L"cbuffer already set for the specified frequency");
	m_cbufferMapByFrequency[frequency] = std::move(cbuffer);
}


void Shader::AddSampler(SamplerUsage usage, std::shared_ptr<Sampler> sampler)
{
	XTEST_ASSERT(m_samplerMapByUsage.find(usage) == m_samplerMapByUsage.end(), L"sampler already set for the specified slot");
	m_samplerMapByUsage[usage] = sampler;
}


CBufferBase* Shader::GetConstantBuffer(CBufferFrequency freqency)
{
	return m_cbufferMapByFrequency.at(freqency).get();
}


void Shader::Init()
{
	for (CBufferMap::reference frequencyPairWithCBuffer : m_cbufferMapByFrequency)
	{
		frequencyPairWithCBuffer.second->Init();
	}
	
	for (SamplerMap::reference slotPairWithSampler : m_samplerMapByUsage)
	{
		slotPairWithSampler.second->Init();
	}
}

