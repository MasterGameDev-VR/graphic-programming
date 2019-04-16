struct VertexIn
{
	float3 posL : POSITION;
	float3 normal: NORMAL;
	float3 tangent : TANGENT;
	float2 uv: UV;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 normalW : NORMALW;
	float3 posW : POSITIONW;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_INV_T;
	float4x4 WVP;
	Material material;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = mul(float4(vin.posL, 1.0), WVP);
	// calculate normalW form vin.normal
	vout.normalW = mul(vin.normal, (float3x3)W_INV_T);
	// calculate posW form vin.posL
	vout.posW = mul(vin.posL, (float3x3)W);
	return vout;
}
