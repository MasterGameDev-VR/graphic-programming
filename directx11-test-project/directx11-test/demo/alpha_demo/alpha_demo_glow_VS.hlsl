
struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	float4x4 WVPT_shadowMap;
	Material material;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = mul(float4(vin.posL, 1.0f), WVP);
	vout.uv = mul(float4(vin.uv, 0.f, 1.f), TexcoordMatrix).xy;

	return vout;
}