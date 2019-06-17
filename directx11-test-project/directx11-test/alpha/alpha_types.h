#pragma once
#include <render/renderable.h>

namespace alpha
{
	struct TextureVertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
	};

	struct GlowObject
	{
		
		Microsoft::WRL::ComPtr<ID3D11Resource> glowTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glowTextureView;
		
	};
} 