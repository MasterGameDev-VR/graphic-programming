#define SAMPLE_COUNT 256

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct VertexInAmbientOcclusion
{
	float3 posL : POSITION;
	float3 toFarPlaneIndex : NORMAL;
	float2 uv : TEXCOORD;
};

struct VertexOutAmbientOcclusion
{
	float4 posH : SV_POSITION;
	float3 toFarPlane : TEXCOORD0;
	float2 uv : TEXCOORD1;
};


cbuffer PerObjectCBAmbientOcclusion : register(b3)
{
	float4x4 viewToTexSpace; // Proj*Tex
	float4 offsetVectors[SAMPLE_COUNT];
	float4 frustumCorners[4];
	float occlusionRadius;
	float occlusionFadeStart;
	float occlusionFadeEnd;
	float surfaceEpsilon;
};

VertexOutAmbientOcclusion main(VertexInAmbientOcclusion vin)
{
	VertexOutAmbientOcclusion vout;

	vout.posH = float4(vin.posL, 1.0f);
	vout.uv = vin.uv;
	vout.toFarPlane = frustumCorners[vin.toFarPlaneIndex.x].xyz;

	return vout;
}
