#define DIRECTIONAL_LIGHT_COUNT 2
#define MAX_SAMPLES 8

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

SamplerState motionSampler : register(s0);

SamplerState TextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
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
	float blurMultiplier;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useShadowMap;
	bool useGlowMap;
	bool useMotionBlurMap;
	float shadowMapResolution;
}

Texture2D texctureRenderMap : register(t20);
Texture2D bloomTexture : register(t40);

Texture2D motionBlurTexture : register(t3);


float4 main(VertexOut pin) : SV_TARGET
{
	float4 textureColor = texctureRenderMap.Sample(TextureSampler, pin.uv);
	float4 bloomColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[flatten]
	if (useGlowMap)
	{
		bloomColor = bloomTexture.Sample(TextureSampler, pin.uv);
	}
	float4 finalColorBloom = saturate(textureColor + bloomColor);

	// combine motion blur
	float3 finalColorBlur = texctureRenderMap.Sample(TextureSampler, pin.uv.xy).rgb;
	[flatten]
	if (useMotionBlurMap)
	{
		float w;
		float h;
		texctureRenderMap.GetDimensions(w, h);
		float2 texelSize = float2(1.0f, 1.0f) / float2(w, h);
		float2 screenTexCoords = pin.posH.xy * texelSize;

		float2 vel = motionBlurTexture.Sample(motionSampler, pin.uv).rg;
		vel *= blurMultiplier;

		float speed = length(vel / texelSize);
		int nSamples = clamp(int(speed), 1, MAX_SAMPLES);

		float sum = 0;
		[unroll]
		for (int i = 1; i < nSamples; ++i) {
			float2 offset = vel * (float(i) / float(nSamples - 1) - 0.5f);
			float factor = float(nSamples - i) / float(nSamples - 1);
			sum += factor;
			finalColorBlur += texctureRenderMap.Sample(TextureSampler, pin.uv + offset).rgb * factor;
		}
		finalColorBlur /= sum + 1.0f;
	}

	float4 finalColor = (finalColorBloom + float4(finalColorBlur, 1.0f));

	//return float4(finalColorBlur, 1.0f);
	return finalColor;
}