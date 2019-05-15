#pragma once

#include <render/render_resource.h>
#include <render/shading/cbuffer.h>
#include <render/shading/sampler.h>
#include <render/shading/common_types.h>
#include <file/binary_file.h>


namespace xtest{
namespace render {
namespace shading{

	// shader base class
	class Shader : public RenderResource
	{
	public:

		explicit Shader(const file::BinaryFile* binaryFile);
		Shader(const Shader&) = default;
		Shader(Shader&&) = default;
		Shader& operator=(const Shader&) = default;
		Shader& operator=(Shader&&) = default;

		virtual ~Shader() {};

		virtual void Init() override;

		void AddConstantBuffer(CBufferFrequency frequency, std::unique_ptr<CBufferBase> cbuffer);
		void AddSampler(SamplerUsage usage, std::shared_ptr<Sampler> sampler);
		CBufferBase* GetConstantBuffer(CBufferFrequency freqency);

		virtual void BindTexture(TextureUsage usage, const ID3D11ShaderResourceView* texture) = 0;


	protected:

		typedef std::unordered_map<CBufferFrequency, std::unique_ptr<CBufferBase>> CBufferMap;
		typedef std::unordered_map<SamplerUsage, std::shared_ptr<Sampler>> SamplerMap;

		const file::BinaryFile* m_byteCode;
		CBufferMap m_cbufferMapByFrequency;
		SamplerMap m_samplerMapByUsage;
		

	};

} //shading
} //render
} //xtest