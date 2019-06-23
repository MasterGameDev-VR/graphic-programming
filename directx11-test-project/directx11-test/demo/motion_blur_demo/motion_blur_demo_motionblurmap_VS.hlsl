cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
	float4x4 prevWVP;
};

struct VertexOut {
	float4 posH : SV_POSITION;
	float4 pos : CURRENT_POSITION;
	float4 prevpos : PREV_POSITION;
};

VertexOut main( float3 posL : POSITION )
{
	VertexOut vout;

	vout.posH = mul(float4(posL, 1.0f), WVP);
	vout.pos = mul(float4(posL, 1.0f), WVP);
	vout.prevpos = mul(float4(posL, 1.0f), prevWVP);

	return vout;
}
