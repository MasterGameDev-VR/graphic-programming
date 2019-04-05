
cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
};

struct VertexIn
{
	float3 posL : POSITION;
	float4 color : COLOR;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = mul(float4(vin.posL, 1.0), WVP);
	vout.color = vin.color;
	return vout;
}
