struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};

struct VertexOutAmbientOcclusion
{
	float4 posH : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 toFarPlane : TEXCOORD1;
};


cbuffer PerFrameCBAmbientOcclusion : register(b0)
{
	float4x4 viewToTexSpace; // Proj * Texture
	float4 offsetVectors[14];
	float4 frustumCorners[4];
	float occlusionRadius = 0.5f;
	float occlusionFadeStart = 0.2f;
	float occlusionFadeEnd = 2.0f;
	float surfaceEpsilon = 0.05f;
};

VertexOutAmbientOcclusion main(VertexIn vin)
{
	VertexOutAmbientOcclusion vout;


	vout.posH = float4(vin.posL, 1.0f);
	vout.toFarPlane = frustumCorners[vin.normalL.x].xyz;
	vout.uv = vin.uv;

	return vout;
}