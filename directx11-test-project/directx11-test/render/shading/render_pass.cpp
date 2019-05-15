#include "stdafx.h"
#include "render_pass.h"
#include <service/locator.h>


using namespace xtest::render::shading;



void RenderPass::SetState(std::shared_ptr<RenderPassState> state)
{
	XTEST_ASSERT(!m_state, L"render pass state already set");
	m_state = state;
}


void RenderPass::SetVertexShader(std::shared_ptr<VertexShader> vs)
{
	XTEST_ASSERT(!m_vertexShader, L"vertex shader already set");
	m_vertexShader = vs;
}


void RenderPass::SetPixelShader(std::shared_ptr<PixelShader> ps)
{
	XTEST_ASSERT(!m_pixelShader, L"pixel shader already set");
	m_pixelShader = ps;
}

RenderPassState* RenderPass::GetState()
{
	return m_state.get();
}


VertexShader* RenderPass::GetVertexShader()
{
	return m_vertexShader.get();
}

PixelShader* RenderPass::GetPixelShader()
{
	return m_pixelShader.get();
}

void RenderPass::Bind()
{
	XTEST_ASSERT(m_state);
	XTEST_ASSERT(m_vertexShader);

	m_state->Bind();
	m_vertexShader->Bind();

	if (m_pixelShader)
	{
		m_pixelShader->Bind();
	}
	else
	{
		// disable the pixel shader for the current pipeline
		service::Locator::GetD3DContext()->PSSetShader(nullptr, nullptr, 0);
	}
}


void xtest::render::shading::RenderPass::Init()
{
	m_state->Init();
	m_vertexShader->Init();
	if (m_pixelShader)
	{
		m_pixelShader->Init();
	}
}

