#pragma once

#include <render/shading/vertex_shader.h>
#include <render/shading/pixel_shader.h>
#include <render/shading/render_pass_state.h>
#include <render/renderable.h>
#include <render/renderable.h>


namespace xtest {
namespace render {
namespace shading {

	// class that represents a render pass
	class RenderPass : public RenderResource
	{
	public:

		void SetState(std::shared_ptr<RenderPassState> state);
		void SetVertexShader(std::shared_ptr<VertexShader> vs);
		void SetPixelShader(std::shared_ptr<PixelShader> ps);

		RenderPassState* GetState();
		VertexShader* GetVertexShader();
		PixelShader* GetPixelShader();

		virtual void Init() override;
		virtual void Bind() override;


	private:

		std::shared_ptr<RenderPassState> m_state;
		std::shared_ptr<VertexShader> m_vertexShader;
		std::shared_ptr<PixelShader> m_pixelShader;

	};

} // shading
} // render
} // xtest
