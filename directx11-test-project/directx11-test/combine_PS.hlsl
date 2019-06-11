Texture2D colorTexture : register(t0);
Texture2D motionBlurTexture : register(t3);

SamplerState textureSampler : register(s0);

float4 main(float4 pin : SV_POSITION) : SV_TARGET {
	float2 vel = motionBlurTexture.Sample(textureSampler, pin.xy).rg;

	return float4(vel.x, vel.y, 0.0f, 0.0f);
}