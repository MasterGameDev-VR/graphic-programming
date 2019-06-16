Texture2D colorTexture : register(t0);
Texture2D motionBlurTexture : register(t3);

SamplerState textureSampler : register(s0);

float4 main(float4 pin : SV_POSITION) : SV_TARGET {
	float3 col = colorTexture.Sample(textureSampler, pin.xy).rgb;
	float2 vel = motionBlurTexture.Sample(textureSampler, pin.xy).rg;

	return float4(col.x, col.y, col.z, 1.0f);
}