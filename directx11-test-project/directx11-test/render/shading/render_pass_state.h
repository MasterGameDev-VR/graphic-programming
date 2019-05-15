#pragma once

#include <render/render_resource.h>
#include <render/shading/rasterizer_state.h>

namespace xtest {
namespace render {
namespace shading {

	//class representing a collection of state a render pass needs to function properly
	class RenderPassState : public RenderResource
	{
	public:

		// right now we only support one render target and one view
		RenderPassState(
			D3D11_PRIMITIVE_TOPOLOGY topology,
			const D3D11_VIEWPORT& vieport,
			std::shared_ptr<RasterizerState> rasterizerState,
			ID3D11RenderTargetView* renderTarget,
			ID3D11DepthStencilView* depthView);


		virtual void Init() override;
		virtual void Bind() override;

		void ChangeViewPort(const D3D11_VIEWPORT& vieport);
		void ChangeRenderTargetView(ID3D11RenderTargetView* renderTarget);
		void ChangeDepthStencilView(ID3D11DepthStencilView* depthView);


		void ClearDepthOnly(float value = 1.f);
		void ClearStencilOnly(uint8 value = 0);
		void ClearDepthStencil(float depthValue = 1.f, uint8 stencilValue = 0.f);
		void ClearRenderTarget(const DirectX::XMFLOAT4& color);
		void ClearRenderTarget(const DirectX::XMVECTORF32& color);


	protected:

		D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
		D3D11_VIEWPORT m_viewport;
		ID3D11RenderTargetView* m_renderTargetView;
		ID3D11DepthStencilView* m_depthStencilView;
		std::shared_ptr<RasterizerState> m_rasterizerState;

	};

} //shading
} //render
} //xtest

