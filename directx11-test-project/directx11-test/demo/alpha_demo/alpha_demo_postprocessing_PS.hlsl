struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

SamplerState TextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useShadowMap;
	bool useGlowMap;
	float shadowMapResolution;
}

Texture2D texctureRenderMap : register(t20);
Texture2D bloomTexture : register(t40);


float4 main(VertexOut pin) : SV_TARGET
{
	float4 textureColor = texctureRenderMap.Sample(TextureSampler, pin.uv);
	float4 bloomColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[flatten]
	if (useGlowMap)
	{
		bloomColor = bloomTexture.Sample(TextureSampler, pin.uv);
	}
	float4 finalColor = saturate(textureColor + bloomColor);

	return finalColor;
}