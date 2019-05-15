#pragma once

#include <render/render_resource.h>
#include <render/shading/common_types.h>
#include <service/locator.h>

namespace xtest {
namespace render {
namespace shading {


	// class that represents a texture sampler, all the implementations should be added to sampler_types.h file
	class Sampler : public RenderResource
	{
	public:

		Sampler() : m_d3dTextureSampler(nullptr), m_target(ShaderTarget::unknown), m_usage(SamplerUsage::unknown) {}
		virtual ~Sampler(){}
		
		void SetUsage(SamplerUsage usage) { m_usage = usage; }
		void SetBindingTarget(ShaderTarget target) { m_target = target; };

		virtual void Bind() override
		{
			XTEST_ASSERT(m_d3dTextureSampler, L"uninitialized sampler");
			XTEST_ASSERT(m_usage != SamplerUsage::unknown);

			switch (m_target)
			{
			case ShaderTarget::vertex_shader:
				service::Locator::GetD3DContext()->VSSetSamplers(static_cast<int>(m_usage), 1, m_d3dTextureSampler.GetAddressOf());
				break;

			case ShaderTarget::pixel_shader:
				service::Locator::GetD3DContext()->PSSetSamplers(static_cast<int>(m_usage), 1, m_d3dTextureSampler.GetAddressOf());
				break;

			default:
				XTEST_ASSERT(false, L"unknown binding point");
				break;
			}
		}

	protected:

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_d3dTextureSampler;
		ShaderTarget m_target;
		SamplerUsage m_usage;

	};


} // shading
} // render
} // xtest

