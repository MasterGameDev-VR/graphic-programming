#define DIRECTIONAL_LIGHT_COUNT 2
#define TARGET_FPS 60

Texture2D colorTexture : register(t0);
Texture2D motionBlurTexture : register(t3);

SamplerState textureSampler : register(s0);

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

struct DirectionalLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dirW;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLights[DIRECTIONAL_LIGHT_COUNT];
	float3 eyePosW;
	float fps;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useShadowMap;
	float shadowMapResolution;
	bool useMotionBlurMap;
}

float4 main(VertexOut pin) : SV_TARGET {
	float3 finalColor = colorTexture.Sample(textureSampler, pin.uv.xy).rgb;

	if (useMotionBlurMap)
	{
		float w;
		float h;
		colorTexture.GetDimensions(w, h);
		float2 texelSize = float2(1.0f, 1.0f) / float2(w, h);
		float2 screenTexCoords = pin.pos.xy * texelSize;

		float2 vel = motionBlurTexture.Sample(textureSampler, pin.uv.xy).rg;
		vel *= (fps / TARGET_FPS);

		float speed = length(vel / texelSize);
		int nSamples = clamp(int(speed), 1, 5);

		[unroll]
		for (int i = 1; i < nSamples; ++i) {
			float2 offset = vel * (float(i) / float(nSamples - 1) - 0.5);
			finalColor += colorTexture.Sample(textureSampler, pin.uv.xy + offset).rgb;
		}
		finalColor /= float(nSamples);
	}

	return float4(finalColor, 1.0f);
}