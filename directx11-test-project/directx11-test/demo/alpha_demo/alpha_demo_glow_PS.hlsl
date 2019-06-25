#define GLOWCONSTANT 10

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};


cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	bool useGlow;
};

Texture2D diffuseTexture : register(t0);
Texture2D glowMap : register(t30);

SamplerState textureSampler : register(s0);




float4 main(VertexOut pin) : SV_TARGET
{
	float4 finalColor = float4(0.f, 0.f, 0.f, 0.f);
	if (useGlow)
	{
		finalColor = diffuseTexture.Sample(textureSampler, pin.uv);
		float glowFactor = glowMap.Sample(textureSampler, pin.uv).r * GLOWCONSTANT;

		finalColor *= glowFactor;
	}

	return finalColor;

}