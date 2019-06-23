#pragma once
#include <render/renderable.h>

namespace xtest {
	namespace render {

		class RenderableInMotion : public Renderable
		{
		public:
			RenderableInMotion(const mesh::GPFMesh& mesh);
			~RenderableInMotion();

			void SetTransformPreviousFrame(const DirectX::FXMMATRIX& W);
			const DirectX::XMFLOAT4X4& GetTransformPreviousFrame() const;

		private:
			DirectX::XMFLOAT4X4 m_W_Previous_Frame;

		};

	}
}