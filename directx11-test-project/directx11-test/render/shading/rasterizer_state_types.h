#pragma once

#include <render/shading/rasterizer_state.h>
#include <service/locator.h>

namespace xtest {
namespace render {
namespace shading {


	/************************************************************************/
	/* house of all the rasterizer state implementations used               */
	/************************************************************************/


	class SolidCullBackRS : public RasterizerState
	{
	public:
		
		virtual void Init() override
		{
			// already initialized
			if (m_d3dRasterizerState)
			{
				return;
			}

		 	D3D11_RASTERIZER_DESC rasterizerDesc;
			ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.FrontCounterClockwise = false;
			rasterizerDesc.DepthClipEnable = true;

			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, &m_d3dRasterizerState));
		}
	};


	


} // shading
} // render
} // xtest