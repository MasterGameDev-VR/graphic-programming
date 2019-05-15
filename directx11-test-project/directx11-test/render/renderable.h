#pragma once

#include <mesh/mesh_format.h>
#include <render/render_resource.h>
#include <render/shading/common_types.h>

namespace xtest {
namespace render {

	// class used to render a geometry starting from a GPFMesh or MeshData.
	// warning: when used with a MeshData a GPFMesh wrapper is created with mesh name equal to ""
	class Renderable : public RenderResource
	{
	public:

		Renderable(const mesh::GPFMesh& mesh = mesh::GPFMesh());

		Renderable(
			const mesh::MeshData& mesh,
			const mesh::MeshMaterial& material
		);

		virtual void Init() override;
		virtual void Bind() override;

		void SetTransform(const DirectX::FXMMATRIX& W);
		void SetTexcoordTransform(const DirectX::FXMMATRIX& T, const std::string& meshName = "");

		const DirectX::XMFLOAT4X4& GetTransform() const;
		const DirectX::XMFLOAT4X4& GetTexcoordTransform(const std::string& meshName = "") const;
		const mesh::MeshMaterial& GetMaterial(const std::string& meshName = "") const;
		const ID3D11ShaderResourceView* GetTextureView(shading::TextureUsage textureUsage, const std::string& meshName = "") const;
		const std::vector<std::string>& GetMeshNames() const;

		void Draw(const std::string& meshName = "");

	private:

		typedef std::unordered_map<shading::TextureUsage, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> TextureViewMapByUsage;
		typedef std::unordered_map<std::string, TextureViewMapByUsage> TextureViewMap;
		typedef std::unordered_map<std::string, DirectX::XMFLOAT4X4> TexcoordMatrixMap;
		
		mesh::GPFMesh m_mesh;
		DirectX::XMFLOAT4X4 m_W;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dIndexBuffer;
		TextureViewMap m_textureMapByUsageMapByMeshName;
		TexcoordMatrixMap m_texcoordMatrixMapByMeshName;
		std::vector<std::string> m_meshNames;
	};

} // render
} // xtest
