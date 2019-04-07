
cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
};

struct Vertex
{

	float3 posL: POSITION;
	float3 normal: NORMAL;
	float3 tangentu: TANGENTU;
	float2 uv: UV
}
struct VertexIn
{
	Vertex vertex : VERTEX;
	
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

	vout.posH = mul(float4(vin.vertex.posL, 1.0), WVP);
	vout.color = vin.color;
	return vout;
}
