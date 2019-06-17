Texture2D colorTexture : register(t0);
Texture2D motionBlurTexture : register(t3);

SamplerState textureSampler : register(s0);

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET {
	float3 col = colorTexture.Sample(textureSampler, pin.uv.xy).rgb;
	float2 vel = motionBlurTexture.Sample(textureSampler, pin.uv.xy).rg;

	return float4(col.x, col.y, col.z, 1.0f);
}