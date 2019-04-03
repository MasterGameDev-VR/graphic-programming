cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
};

struct VertexIn
{
	float3 posL : POSITION;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
	float2 uv : UV;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
	float2 uv : UV;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = mul(float4(vin.posL, 1.0), WVP);
	vout.normal = vin.normal;
	vout.tangentU = vin.tangentU;
	vout.uv = vin.uv;
	return vout;
}
