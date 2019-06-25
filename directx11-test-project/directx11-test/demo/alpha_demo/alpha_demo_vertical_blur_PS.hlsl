

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer PerFrameCB : register(b1)
{
	float horizontalResolution;
};

Texture2D glowTexture : register(t50);
SamplerState TextureSampler : register(s11);

#define WEIGHTSNUMBER 9
#define WEIGHT0 0.13298f	
#define WEIGHT1 0.125858f	
#define WEIGHT2 0.106701f	
#define WEIGHT3 0.081029f	
#define WEIGHT4 0.055119f	
#define WEIGHT5 0.033585f	
#define WEIGHT6 0.018331f	
#define WEIGHT7 0.008962f	
#define WEIGHT8 0.003924f	

float4 main(VertexOut pin) : SV_TARGET
{
	float4 color = float4(0.f, 0.f, 0.f, 0.f);
	// Create the weights that each neighbor pixel will contribute to the blur.
	float weights[WEIGHTSNUMBER] = { WEIGHT0, WEIGHT1, WEIGHT2, WEIGHT3, WEIGHT4, WEIGHT5, WEIGHT6, WEIGHT7, WEIGHT8 };
	int i;
	
	color += glowTexture.Sample(TextureSampler, pin.uv) * weights[0];
	[unroll]
	for (i = 1; i < WEIGHTSNUMBER; i++)
	{
		color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +i / horizontalResolution)) * weights[i];
		color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -i / horizontalResolution)) * weights[i];
	}

	// Set the alpha channel to one.
	color.a = 1.0f;

	return color;
}