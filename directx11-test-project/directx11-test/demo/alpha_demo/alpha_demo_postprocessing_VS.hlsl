struct VertexIn
{
	float3 posL : POSITION;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posH = float4(vin.posL, 1.0f);
	vout.uv = vin.uv;

	return vout;
}