#pragma once
#include <render/shading/shader.h>


namespace xtest {
namespace render {
namespace shading {

	// pixel shader implementation
	class PixelShader : public Shader
	{
	public:

		using Shader::Shader;	

		virtual void Init() override;
		virtual void Bind() override;
		void BindTexture(TextureUsage usage, const ID3D11ShaderResourceView* texture) override;
	
	protected:

		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

	};


} // shading
} // render
} // xtest

