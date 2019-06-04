#pragma once

#include <render/render_resource.h>

namespace alpha
{
	class TextureRenderable : public xtest::render::RenderResource
	{
	public:
		virtual void Init() override;
		virtual void Bind() override;
		//void Update(float textureWidth, float textureHieght, float positionX = 0, float positionY = 0);
		void Draw();


	private:
		uint32 m_vertexCount;
		uint32 m_indexCount;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dIndexBuffer;
	};

	
}