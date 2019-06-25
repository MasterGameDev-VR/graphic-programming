struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

//INPUT to the Normal/Depth PS
struct VertexOutNormalDepth
{
	float4 posH : SV_POSITION;
	float3 posV : POSITION;
	float3 normalV : NORMAL;
	float2 uv : TEXCOORD0;
};

float4 main(VertexOutNormalDepth pin) : SV_TARGET
{
	//renormalize interpolated normals.
	pin.normalV = normalize(pin.normalV);

	return float4(pin.normalV, pin.posV.z); 
}