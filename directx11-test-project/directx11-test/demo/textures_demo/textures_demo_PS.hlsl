
#define POINT_LIGHT_COUNT 4
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

struct PointLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 attenuation;
};

struct SpotLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 dirW;
	float spot;
	float3 attenuation;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
};



cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLights[DIRECTIONAL_LIGHT_COUNT];
	PointLight pointLights[POINT_LIGHT_COUNT];
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useBumpMap;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D glossTexture : register(t2);
SamplerState textureSampler : register(s0);






float4 CalculateAmbient(float4 matAmbient, float4 lightAmbient)
{
	return matAmbient * lightAmbient;
}


void CalculateDiffuseAndSpecular(
	float3 toLightW,
	float3 normalW,
	float3 toEyeW,
	float4 matDiffuse,
	float4 matSpec,
	float4 lightDiffuse,
	float4 lightSpec,
	float glossSample,
	inout float4 diffuseColor,
	inout float4 specularColor)
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


float3 BumpNormalW(float2 uv, float3 normalW, float3 tangentW)
{
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;

	// remap the normal values inside the [-1,1] range from the [0,1] range
	float3 bumpNormalT = 2.f * normalSample - 1.f;

	// create the tangent space to world space matrix
	float3x3 TBN = float3x3(tangentW, cross(normalW, tangentW), normalW);

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


void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, float glossSample, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);


	float distance;
	float3 toLightW;
	CalculateDirAndDistance(posW, light.posW, toLightW, distance);

	
	// ealry rejection
	if (distance > light.range)
		return;
	

	float falloff = 1.f - (distance / light.range);
	
	// shading componets
	ambient = CalculateAmbient(mat.ambient, light.ambient);
	CalculateDiffuseAndSpecular(toLightW, normalW, toEyeW, mat.diffuse, mat.specular, light.diffuse, light.specular, glossSample, diffuse, specular);
	ApplyAttenuation(light.attenuation, distance, falloff, ambient, diffuse, specular);
}


void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, float glossSample, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float distance;
	float3 toLightW;
	CalculateDirAndDistance(posW, light.posW, toLightW, distance);


	// ealry rejection
	if (distance > light.range)
		return;


	// spot effect factor
	float spot = pow(max(dot(-toLightW, light.dirW), 0.0f), light.spot);

	// shading componets
	ambient = CalculateAmbient(mat.ambient, light.ambient);
	CalculateDiffuseAndSpecular(toLightW, normalW, toEyeW, mat.diffuse, mat.specular, light.diffuse, light.specular, glossSample, diffuse, specular);
	ApplyAttenuation(light.attenuation, distance, spot, ambient, diffuse, specular);
}




float4 main(VertexOut pin) : SV_TARGET
{
	pin.normalW = normalize(pin.normalW);

	// make sure tangentW is still orthogonal to normalW and is unit leght even
	// after the rasterizer stage (interpolation) 
	pin.tangentW = pin.tangentW - (dot(pin.tangentW, pin.normalW)*pin.normalW);
	pin.tangentW = normalize(pin.tangentW);


	// bump normal from texture
	float3 bumpNormalW;

	if (useBumpMap)
	{
		bumpNormalW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);
	}
	else
	{
		bumpNormalW = pin.normalW;
	}


	float glossSample = glossTexture.Sample(textureSampler, pin.uv).r;
	float3 toEyeW = normalize(eyePosW - pin.posW);
	
	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);
	float4 ambient;
	float4 diffuse;
	float4 specular;


	if (useDirLight)
	{
		[unroll]
		for (uint i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
		{
			DirectionalLightContribution(material, dirLights[i], bumpNormalW, toEyeW, glossSample, ambient, diffuse, specular);
			totalAmbient += ambient;
			totalDiffuse += diffuse;
			totalSpecular += specular;
		}
	}


	if (usePointLight)
	{
		[unroll]
		for (uint i = 0; i < POINT_LIGHT_COUNT; i++)
		{
			PointLightContribution(material, pointLights[i], pin.posW, bumpNormalW, toEyeW, glossSample, ambient, diffuse, specular);
			totalAmbient += ambient;
			totalDiffuse += diffuse;
			totalSpecular += specular;
		}

	}
	

	float4 diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);
	float4 finalColor = diffuseColor * (totalAmbient + totalDiffuse) + totalSpecular;
	finalColor.a = diffuseColor.a * totalDiffuse.a;

	return finalColor;

}