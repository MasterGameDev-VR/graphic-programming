
#define DIRECTIONAL_LIGHT_COUNT 2


struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct DirectionalLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dirW;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float4 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	float4 shadowPosH : SHADOWPOS;
	float4 ssaoPosH : TEXCOORD1;
};



cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 WVPT;
	float4x4 TexcoordMatrix;
	float4x4 WVPT_shadowMap;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLights[DIRECTIONAL_LIGHT_COUNT];
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useShadowMap;
	float shadowMapResolution;
	bool useSSAOMap;
	float SSOAMultiplier=1;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D glossTexture : register(t2);

Texture2D shadowMap : register(t10);
Texture2D ssaoMap : register(t13);

SamplerState textureSampler : register(s0);
SamplerState SSAOSampler : register(s3)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState shadowSampler : register(s10);

float4 CalculateAmbient(float4 matAmbient, float4 lightAmbient)
{
	return matAmbient * lightAmbient;
}

void CalculateDiffuseAndSpecular (
	float3 toLightW,
	float3 normalW,
	float3 toEyeW,
	float4 matDiffuse,
	float4 matSpec,
	float4 lightDiffuse,
	float4 lightSpec,
	float glossSample,
	inout float4 diffuseColor,
	inout float4 specularColor )
{

	// diffuse factor
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.0f)
	{
		// diffuse component
		diffuseColor = Kd * matDiffuse * lightDiffuse;

		// specular component
		float3 halfVectorW = normalize(toLightW + toEyeW);
		float exponent = exp2(9.0 * glossSample);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		specularColor = Ks * matSpec * lightSpec;
	}
}


void ApplyAttenuation(
	float3 lightAttenuation,
	float distance,
	float falloff,
	inout float4 ambient,
	inout float4 diffuse,
	inout float4 specular)
{
	float attenuationFactor = 1.0f / dot(lightAttenuation, float3(1.0f, distance, distance*distance));
	ambient *= falloff;
	diffuse *= attenuationFactor * falloff;
	specular *= attenuationFactor * falloff;
}


void CalculateDirAndDistance(float3 pos, float3 target, out float3 dir, out float distance)
{
	float3 toTarget = target - pos;
	distance = length(toTarget);

	// now dir is normalize 
	toTarget /= distance;

	dir = toTarget;
}


float3 BumpNormalW(float2 uv, float3 normalW, float4 tangentW)
{
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;

	// remap the normal values inside the [-1,1] range from the [0,1] range
	float3 bumpNormalT = 2.f * normalSample - 1.f;

	// create the tangent space to world space matrix
	float3x3 TBN = float3x3(tangentW.xyz, tangentW.w * cross(normalW, tangentW.xyz), normalW);

	return mul(bumpNormalT, TBN);
}




void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, float glossSample, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);


	float3 toLightW = -light.dirW;

	// shading componets
	ambient = CalculateAmbient(mat.ambient, light.ambient);
	CalculateDiffuseAndSpecular(toLightW, normalW, toEyeW, mat.diffuse, mat.specular, light.diffuse, light.specular, glossSample, diffuse, specular);

}



float4 main(VertexOut pin) : SV_TARGET
{
	pin.normalW = normalize(pin.normalW);
	
	float ambientAccess = 1.0f;

	if (useSSAOMap)
	{
		pin.ssaoPosH /= pin.ssaoPosH.w;
		ambientAccess = ssaoMap.Sample(SSAOSampler, pin.ssaoPosH.xy, 0.0f).r;
	}

	// make sure tangentW is still orthogonal to normalW and is unit leght even
	// after the rasterizer stage (interpolation) 
	pin.tangentW.xyz = pin.tangentW.xyz - (dot(pin.tangentW.xyz, pin.normalW)*pin.normalW);
	pin.tangentW.xyz = normalize(pin.tangentW.xyz);


	// bump mapping
	float3 bumpNormalW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);


	float litFactor = 1.f;
	if (useShadowMap)
	{
		litFactor = 0.f;

		pin.shadowPosH.xyz /= pin.shadowPosH.w;
		float depthNDC = pin.shadowPosH.z;

		const float delta = 1.f / shadowMapResolution;


		const float2 texelOffset[9] =
		{
			float2(0.f, delta),
			float2(delta, delta),
			float2(delta, 0.f),
			float2(delta, -delta),
			float2(0.f, -delta),
			float2(-delta, -delta),
			float2(-delta, 0.f),
			float2(-delta, +delta),
			float2(0.f, 0.f),
		};


		[unroll]
		for (int i = 0; i < 9; i++)
		{
			litFactor += shadowMap.SampleCmpLevelZero(shadowSampler, pin.shadowPosH.xy + texelOffset[i], depthNDC).r;
		}

		litFactor /= 9;
	}


	float glossSample = glossTexture.Sample(textureSampler, pin.uv).r;
	float3 toEyeW = normalize(eyePosW - pin.posW);

	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);
	float4 ambient;
	float4 diffuse;
	float4 specular;


	// we want to apply our shadow map only to the directional key light
	float litFactors[2] = { litFactor, 1.f };

	[unroll]
	for (uint i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
	{
		DirectionalLightContribution(material, dirLights[i], bumpNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		
		totalAmbient += ambient;
		
	
		totalDiffuse += diffuse * litFactors[i];
		totalSpecular += specular * litFactors[i];
	}

	totalAmbient *= ambientAccess;

	float4 diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);
	float4 finalColor = diffuseColor * (totalAmbient + totalDiffuse) + totalSpecular;
	finalColor.a = diffuseColor.a * totalDiffuse.a;

	return finalColor;
}