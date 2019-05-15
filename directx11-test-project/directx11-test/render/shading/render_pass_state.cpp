#include "stdafx.h"
#include "render_pass_state.h"
#include <service/locator.h>


using namespace xtest::render::shading;


RenderPassState::RenderPassState(
	D3D11_PRIMITIVE_TOPOLOGY topology,
	const D3D11_VIEWPORT& viewport,
	std::shared_ptr<RasterizerState> rasterizerState,
	ID3D11RenderTargetView* renderTarget,
	ID3D11DepthStencilView* depthView
)
	: m_primitiveTopology(topology)
	, m_viewport(viewport)
	, m_rasterizerState(rasterizerState)
	, m_renderTargetView(renderTarget)
	, m_depthStencilView(depthView)
{}


void RenderPassState::Bind()
{
	XTEST_ASSERT(m_depthStencilView);

	ID3D11DeviceContext* d3dContext = service::Locator::GetD3DContext();
	d3dContext->IASetPrimitiveTopology(m_primitiveTopology);
	d3dContext->RSSetViewports(1, &m_viewport);
	d3dContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	m_rasterizerState->Bind();	
}


void RenderPassState::ChangeViewPort(const D3D11_VIEWPORT& vieport)
{
	m_viewport = vieport;
}


void RenderPassState::ChangeRenderTargetView(ID3D11RenderTargetView* renderTarget)
{
	m_renderTargetView = renderTarget;
}


void RenderPassState::ChangeDepthStencilView(ID3D11DepthStencilView* depthView)
{
	XTEST_ASSERT(depthView);
	m_depthStencilView = depthView;
}


void RenderPassState::ClearDepthOnly(float value)
{
	XTEST_ASSERT(m_depthStencilView);
	service::Locator::GetD3DContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, value, 0);
}


void RenderPassState::Init()
{
	m_rasterizerState->Init();
}


void RenderPassState::ClearStencilOnly(uint8 value)
{
	XTEST_ASSERT(m_depthStencilView);
	service::Locator::GetD3DContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL, 0.f, value);
}


void RenderPassState::ClearDepthStencil(float depthValue, uint8 stencilValue)
{
	XTEST_ASSERT(m_depthStencilView);
	service::Locator::GetD3DContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthValue, stencilValue);
}


void RenderPassState::ClearRenderTarget(const DirectX::XMFLOAT4& color)
{
	XTEST_ASSERT(m_renderTargetView);
	float color_asArray[] = { color.x, color.y, color.z, color.w };
	service::Locator::GetD3DContext()->ClearRenderTargetView(m_renderTargetView, color_asArray);
}


void RenderPassState::ClearRenderTarget(const DirectX::XMVECTORF32& color)
{
	XTEST_ASSERT(m_renderTargetView);
	service::Locator::GetD3DContext()->ClearRenderTargetView(m_renderTargetView, color);
}
