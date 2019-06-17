struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

cbuffer PerObjectCBNormalDepth : register(b4)
{
	float4x4 worldView;
	float4x4 worldInvTransposeView;
	float4x4 worldViewProj;
	float4x4 texTransform;
};

//INPUT to the Normal/Depth PS
struct VertexOutNormalDepth
{
	float4 posH : SV_POSITION;
	float3 posV : POSITION;
	float3 normalV : NORMAL;
	float2 uv : TEXCOORD0;
};

VertexOutNormalDepth main(VertexIn vin)
{
	VertexOutNormalDepth vout;

	vout.posV = mul(float4(vin.posL, 1.0f), worldView).xyz;
	vout.normalV = mul(vin.normalL, (float3x3)worldInvTransposeView);

	// Transform to homogeneous clip space.
	vout.posH = mul(float4(vin.posL, 1.0f), worldViewProj);

	// Output vertex attributes for interpolation across triangle.
	vout.uv = mul(float4(vin.uv, 0.0f, 1.0f), texTransform).xy;

	return vout;
}
