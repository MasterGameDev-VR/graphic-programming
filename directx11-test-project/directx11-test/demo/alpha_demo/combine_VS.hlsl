struct VertexIn
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

VertexOut main(VertexIn pin)
{
	VertexOut vout;
	vout.pos = float4(pin.pos, 1.0f);
	vout.uv = pin.uv;
	return vout;
}
