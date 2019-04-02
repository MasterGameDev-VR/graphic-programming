

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 WVP;
	Material material;
};



struct VertexIn
{
	float3 posL : POSITION;
	float3 normal : NORMAL;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normal : NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = mul(float4(vin.posL, 1.0), WVP);
	vout.posW = (mul(float4(vin.posL, 1.0), W)).xyz;
	vout.normal = mul(vin.normal, (float3x3)W );
	return vout;
}