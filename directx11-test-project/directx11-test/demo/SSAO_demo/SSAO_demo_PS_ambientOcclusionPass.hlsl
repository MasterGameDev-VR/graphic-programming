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

Texture2D normalDepthMap : register(t0);
Texture2D randomVecMap : register(t1);
SamplerState samNormalDepth : register(s0);
SamplerState samRandomVec : register(s1);

float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > surfaceEpsilon)
	{
		float fadeLength = occlusionFadeEnd - occlusionFadeStart;
		occlusion = saturate((occlusionFadeEnd - distZ)/fadeLength);
	}
	return occlusion;
}

float4 main(VertexOutAmbientOcclusion pin, uniform int sampleCount) : SV_TARGET
{
	float4 normalDepth = normalDepthMap.SampleLevel(samNormalDepth, pin.uv, 0.0f);
	float3 n = normalDepth.xyz;
	float pz = normalDepth.w;

	float3 p = (pz / pin.toFarPlane.z)*pin.toFarPlane;

	float3 randVec = 2.0f*randomVecMap.SampleLevel(samRandomVec, 4.0f*pin.uv, 0.0f).rgb - 1.0f;
	float occlusionSum = 0.0f;

	[unroll]
	for (int i = 0; i < sampleCount; ++i)
	{
		float3 offset = reflect(offsetVectors[i].xyz, randVec);
		float flip = sign(dot(offset, n));
		float3 q = p + flip * occlusionRadius * offset;
		float4 projQ = mul(float4(q, 1.0f), viewToTexSpace);
		projQ /= projQ.w;
		float rz = normalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;
		float3 r = (rz / q.z)*q;
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}
	occlusionSum /= sampleCount;
	float access = 1.0f - occlusionSum;
	return saturate(pow(access, 4.0f));
}