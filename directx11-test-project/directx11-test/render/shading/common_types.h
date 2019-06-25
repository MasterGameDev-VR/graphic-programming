#pragma once

namespace xtest {
	namespace render {
		namespace shading {

			// used to bind a cbuffer to a shader 
			// warning: the value of the enum is used to select the shader slot
			enum class CBufferFrequency
			{
				per_object = 0,
				per_frame = 1,
				rarely_changed = 2,
				per_object_ambient_occlusion = 3,
				per_frame_normal_depth = 4,
				per_frame_blur = 5,
				unknown

			};


			// used to bind a resource to the corrisponding shader
			enum class ShaderTarget
			{
				vertex_shader,
				pixel_shader,
				unknown
			};


	// used to bind a texture to a shader
	// warning: the value of the enum is used to select the shader slot
	enum class TextureUsage
	{
		color = 0,
		normal = 1,
		glossiness = 2,
		motionblur=3,
		shadow_map = 10,
		normal_depth_map = 11,
		light_occlusion_map = 12,
		ssao_map = 13,
		texture_map = 20,
		glow_map = 30,
		bloom = 40,
		blur = 50,
		scaleSample = 60,
		uknown
	};

	// used to bind a texture sampler to a shader
	// warning: the value of the enum is used to select the shader slot
	enum class SamplerUsage
	{
		common_textures = 0,
		normal_depth_map = 1,
		random_vec = 2,
		ssao_map = 3,
		ssao_blur = 4,
		shadow_map = 10,
		blur = 11,
		light_occlusion_map = 12,
		texture_map = 13,
		unknown
	};


		} // shading
	} // render
} // xtest
