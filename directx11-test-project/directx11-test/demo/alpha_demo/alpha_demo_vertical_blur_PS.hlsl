
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
SamplerState TextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


float4 main(VertexOut pin) : SV_TARGET
{
	float4 color = float4(0.f, 0.f, 0.f, 0.f);
	float weight0, weight1, weight2, weight3, weight4, weight5;
	float normalization;

	// Create the weights that each neighbor pixel will contribute to the blur.
	weight0 = 1.0f;
	weight1 = 0.9f;
	weight2 = 0.55f;
	weight3 = 0.40f;
	weight4 = 0.18f;
	weight5 = 0.1f;

	// Create a normalized value to average the weights out a bit.
	normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

	// Normalize the weights.
	weight0 = weight0 / normalization;
	weight1 = weight1 / normalization;
	weight2 = weight2 / normalization;
	weight3 = weight3 / normalization;
	weight4 = weight4 / normalization;
	weight5 = weight5 / normalization;
	
	// Add the nine horizontal pixels to the color by the specific weight of each.
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +12 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +11 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +10 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +9 / horizontalResolution)) * weight3;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +8 / horizontalResolution)) * weight4;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +7 / horizontalResolution)) * weight4;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +6 / horizontalResolution)) * weight3;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +5 / horizontalResolution)) * weight5;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +4 / horizontalResolution)) * weight4;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +3 / horizontalResolution)) * weight3;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +2 / horizontalResolution)) * weight2;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, +1 / horizontalResolution)) * weight1;
	color += glowTexture.Sample(TextureSampler, pin.uv) * weight0;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -1 / horizontalResolution)) * weight1;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -2 / horizontalResolution)) * weight2;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -3 / horizontalResolution)) * weight3;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -4 / horizontalResolution)) * weight4;
	color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -5 / horizontalResolution)) * weight5;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -6 / horizontalResolution)) * weight3;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -7 / horizontalResolution)) * weight4;
	//color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -8 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -9 / horizontalResolution)) * weight3;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -10 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -11 / horizontalResolution)) * weight4;
	// color += glowTexture.Sample(TextureSampler, pin.uv + float2(0.0f, -12 / horizontalResolution)) * weight4;

	// Set the alpha channel to one.
	color.a = 1.0f;

	return color;
}