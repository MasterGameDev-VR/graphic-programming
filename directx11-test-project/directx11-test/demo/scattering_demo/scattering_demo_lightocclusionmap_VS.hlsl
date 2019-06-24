struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
	Material material;
};

float4 main(VertexIn vin) : SV_POSITION
{
	float4 posH = mul(float4(vin.posL, 1.0f), WVP);

	return posH;
}