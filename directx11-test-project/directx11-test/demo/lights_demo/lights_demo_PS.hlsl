struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 normal : NORMAL;
	float3 tangentU : TANGENT;
	float2 uv : UV;
};

float4 main(VertexOut pin) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}