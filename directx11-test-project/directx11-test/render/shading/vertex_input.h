#pragma once

#include <render/render_resource.h>
#include <service/locator.h>

namespace xtest {
namespace render {
namespace shading {


	// base class for a vertex input, all the implementations should be placed in the vertex_input_types.h file
	class VertexInput : public RenderResource
	{
	public:

		VertexInput() {}
		VertexInput(const VertexInput&) = default;
		VertexInput(VertexInput&&) = default;
		VertexInput& operator=(const VertexInput&) = default;
		VertexInput& operator=(VertexInput&&) = default;
		virtual ~VertexInput() {};


		void SetVertexShaderByteCode(const file::BinaryFile* vertexShaderByteCode)
		{
			m_vsByteCode = vertexShaderByteCode;
		}

		virtual void Bind() override
		{
			XTEST_ASSERT(m_d3dInputLayout, L"uninitialized vertex inpput");
			service::Locator::GetD3DContext()->IASetInputLayout(m_d3dInputLayout.Get());
		}

	protected:

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_d3dInputLayout;
		const file::BinaryFile* m_vsByteCode;

	};

}// shading
}// render
}// xtest