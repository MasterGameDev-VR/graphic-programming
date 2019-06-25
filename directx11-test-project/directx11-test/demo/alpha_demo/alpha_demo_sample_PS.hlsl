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

Texture2D targetTexture : register(t60);


float4 main(VertexOut pin) : SV_TARGET
{
	float4 finalColor = targetTexture.Sample(TextureSampler, pin.uv);

	return finalColor;
}