struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

struct VertexOutNormalDepth
{
	float4 posH : SV_POSITION;
	float3 posV : VIEW_POSITION; //
	float3 normalV : VIEW_NORMAL; //
	float2 uv : TEXCOORD;
};


cbuffer PerObjectCBNormalDepth : register(b0)
{
	float4x4 WV; //
	float4x4 W_inverseTrasposeV; //
	float4x4 WVP;
	float4x4 TexcoordMatrix;
};


VertexOutNormalDepth main(VertexIn vin)
{
	VertexOutNormalDepth vout;
	vout.posV = mul(float4(vin.posL, 1.0f), WV).xyz;
	vout.normalV = mul(vin.normalL, (float3x3)W_inverseTrasposeV);
	vout.posH = mul(float4(vin.posL, 1.0f), WVP);
	vout.uv = mul(float4(vin.uv, 0.f, 1.f), TexcoordMatrix).xy;
	return vout;
}