#include "stdafx.h"
#include "renderable_in_motion.h"

using namespace xtest;
using xtest::render::RenderableInMotion;

RenderableInMotion::RenderableInMotion(const mesh::GPFMesh& mesh)
	: Renderable(mesh)
	, m_W_Previous_Frame()
{
}


RenderableInMotion::~RenderableInMotion()
{
}

void RenderableInMotion::SetTransformPreviousFrame(const DirectX::FXMMATRIX& W_Previous_Frame) {
	XMStoreFloat4x4(&m_W_Previous_Frame, W_Previous_Frame);
}
const DirectX::XMFLOAT4X4& RenderableInMotion::GetTransformPreviousFrame() const {
	return m_W_Previous_Frame;
}