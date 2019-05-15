#pragma once
#include <render/shading/shader.h>
#include <render/shading/vertex_input.h>

namespace xtest {
namespace render {
namespace shading {

	// class representing a vertex shader
	class VertexShader : public Shader
	{
	public:

		using Shader::Shader;	

		virtual void Init() override;
		virtual void Bind() override;
		virtual void BindTexture(TextureUsage usage, const ID3D11ShaderResourceView* texture) override;

		void SetVertexInput(std::shared_ptr<VertexInput> vertexInput);

	protected:

		std::shared_ptr<VertexInput> m_vertexInput;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;
		

	};


} // shader
} // render
} // xtest

