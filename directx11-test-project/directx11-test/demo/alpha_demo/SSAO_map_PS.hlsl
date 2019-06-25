#define SAMPLE_COUNT 256

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useShadowMap;
	float shadowMapResolution;
	bool useSSAOMap;
	float SSOAMultiplier = 4;
}

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

Texture2D normalDepthMap : register(t11);

SamplerState samNormalDepth : register(s1)
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};

float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > surfaceEpsilon)
	{
		float fadeLength = occlusionFadeEnd - occlusionFadeStart;
		occlusion = saturate((occlusionFadeEnd - distZ ) / fadeLength);
	}
	return occlusion;
}


float main(VertexOutAmbientOcclusion pin) : SV_TARGET
{
	float4 normalDepth = normalDepthMap.SampleLevel(samNormalDepth, pin.uv, 0.0f);
	float3 n = normalDepth.xyz;
	float pz = normalDepth.w;
	float3 p = (pz / pin.toFarPlane.z)*pin.toFarPlane;
	float3 randVec = 1;
	float occlusionSum = 0.0f;

	[unroll]
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		float3 offset = reflect(offsetVectors[i].xyz, randVec);
		float flip = sign(dot(offset, n));
		float3 q = p + flip * occlusionRadius*offset;
		float4 projQ = mul(float4(q, 1.0f), viewToTexSpace);
		projQ /= projQ.w;
		float rz = normalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;
		float3 r = (rz / q.z)*q;
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}

	occlusionSum /= SAMPLE_COUNT;
	float access = 1.0f - occlusionSum;
	return saturate(pow(access, SSOAMultiplier));
}
