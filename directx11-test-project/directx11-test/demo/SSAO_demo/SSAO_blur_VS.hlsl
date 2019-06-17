

struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

struct VertexOutBlurPass
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD;
};

VertexOutBlurPass main(VertexIn vin)
{
	VertexOutBlurPass vout;
	vout.posH = float4(vin.posL, 1.0f);
	vout.uv = vin.uv;
	return vout;
}