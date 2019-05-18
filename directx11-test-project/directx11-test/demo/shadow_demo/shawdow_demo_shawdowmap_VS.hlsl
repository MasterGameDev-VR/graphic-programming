struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float3 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
};


float4 main(float3 posL : POSITION) : SV_POSITION
{
	return mul(float4(posL, 1.f), WVP);
}