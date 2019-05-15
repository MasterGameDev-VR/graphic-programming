#pragma once

#include <service/locator.h>
#include <render/shading/vertex_input.h>
#include <mesh/mesh_format.h>


namespace xtest {
namespace render {
namespace shading {

	/************************************************************************/
	/* house of all the vertex input implementations used                   */
	/************************************************************************/


	class MeshDataVertexInput : public VertexInput
	{
	public:
		
		virtual void Init() override
		{
			XTEST_ASSERT(m_vsByteCode, L"no vertex shader bytecode set");

			// already initialized
			if (m_d3dInputLayout)
			{
				return;
			}

			D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(mesh::MeshData::Vertex, tangentU), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, offsetof(mesh::MeshData::Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC), m_vsByteCode->Data(), m_vsByteCode->ByteSize(), &m_d3dInputLayout));
		}
	};




}// shading
}// render
}// xtest
