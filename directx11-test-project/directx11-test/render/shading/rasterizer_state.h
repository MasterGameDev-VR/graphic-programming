#pragma once

#include <render/render_resource.h>
#include <service/locator.h>


namespace xtest {
namespace render {
namespace shading {

	// rasterizer state base class, new implementations should be added to rasterizer_state_types.h file
	class RasterizerState : public RenderResource
	{
	public:
		
		virtual ~RasterizerState() {}

		virtual void Bind() override
		{
			XTEST_ASSERT(m_d3dRasterizerState, L"uninitialized rasterizer state");
			service::Locator::GetD3DContext()->RSSetState(m_d3dRasterizerState.Get());
		}
	
	protected:

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_d3dRasterizerState = nullptr;

	};


} // shading
} // render
} // xtest
