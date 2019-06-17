struct VertexOutNormalDepth
{
	float4 posH : SV_POSITION;
	float3 posV : VIEW_POSITION; //
	float3 normalV : VIEW_NORMAL; //
	float2 uv : TEXCOORD;
};


Texture2D diffuseMap : register(t0);
SamplerState textureSampler : register(s0);

float4 main(VertexOutNormalDepth pin) : SV_TARGET
{
	pin.normalV = normalize(pin.normalV);
	float4 texColor = diffuseMap.Sample(textureSampler, pin.uv);
	clip(texColor.a - 0.1f);
	return float4(pin.normalV, pin.posV.z);
}