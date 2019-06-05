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

Texture2D texctureRenderMap : register(t20);
Texture2D bloomTexture : register(t40);


float4 main(VertexOut pin) : SV_TARGET
{
	float4 finalColor = texctureRenderMap.Sample(TextureSampler, pin.uv);
	finalColor += bloomTexture.Sample(TextureSampler, pin.uv);

	return finalColor;
}