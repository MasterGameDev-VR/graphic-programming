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
	float2 uv : TEXCOORD;
	float3 tangentW : TANGENT;
};


Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D glossTexture : register(t2);
SamplerState textureSampler : register(s0);

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	Material material;
	float4x4 texCoord;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLight;
	PointLight pointLightArray[6];
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useSpotLight;
}


// Normal mapping
float3 BumpNormalW(float2 uv, float3 normalW, float3 tangentW)
{
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;

	float3 bumpNormalT = 2.f * normalSample - 1.f;							// from range [0,1] to [-1,1]

	float3x3 TBN = float3x3(tangentW, cross(normalW, tangentW), normalW);	// change base to world space

	return mul(bumpNormalT, TBN);
}


void DirectionalLightContribution(Material mat, DirectionalLight light, float glossSample, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);


	// ambient component
	ambient += mat.ambient * light.ambient;

	// diffuse factor
	float3 toLightW = -light.dirW;
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f)
	{
		// diffuse component
		diffuse += Kd * mat.diffuse * light.diffuse;

		// specular component
		float3 halfVectorW = normalize(toLightW + toEyeW);
		float exponent = exp2(13.0 * (1.f - glossSample));
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		specular += Ks * mat.specular * light.specular;
	}
}



void PointLightContribution(Material mat, PointLight light, float glossSample, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{

	// default values
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLightW = light.posW - posW;
	float distance = length(toLightW);

	// ealry rejection
	if (distance > light.range)
		return;

	// now light dir is normalize 
	toLightW /= distance;


	// ambient component
	ambient = mat.ambient * light.ambient;

	// diffuse factor
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.0f)
	{
		// diffuse component
		diffuse = Kd * mat.diffuse * light.diffuse;

		// specular component
		float3 halfVectorW = normalize(toLightW + toEyeW);
		float exponent = exp2(13.0 * (1.f - glossSample));
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		specular = Ks * mat.specular * light.specular;
	}

	// custom "gentle" falloff
	float falloff = 1.f - (distance / light.range);

	// attenuation
	float attenuationFactor = 1.0f / dot(light.attenuation, float3(1.0f, distance, distance*distance));
	ambient *= falloff;
	diffuse *= attenuationFactor * falloff;
	specular *= attenuationFactor * falloff;

}



void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLightW = light.posW - posW;
	float distance = length(toLightW);

	// ealry rejection
	if (distance > light.range)
		return;

	// now light dir is normalize 
	toLightW /= distance;

	// ambient component
	ambient = mat.ambient * light.ambient;

	// diffuse factor
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.0f)
	{
		// diffuse component
		diffuse = Kd * mat.diffuse * light.diffuse;

		// specular component
		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular = Ks * mat.specular * light.specular;

	}

	// spot effect factor
	float spot = pow(max(dot(-toLightW, light.dirW), 0.0f), light.spot);

	// attenuation
	float attenuationFactor = 1 / dot(light.attenuation, float3(1.0f, distance, distance*distance));
	ambient *= spot;
	diffuse *= spot * attenuationFactor;
	specular *= spot * attenuationFactor;
}


float4 main(VertexOut pin) : SV_TARGET
{
	pin.normalW = normalize(pin.normalW);

	float3 toEyeW = normalize(eyePosW - pin.posW);


	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	// diffuse map
	float4 diffuseTexColor = diffuseTexture.Sample(textureSampler, pin.uv);

	// normal map
	pin.tangentW = pin.tangentW - (dot(pin.tangentW, pin.normalW) * pin.normalW);
	pin.tangentW = normalize(pin.tangentW);
	pin.normalW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);

	// gloss map
	float glossSample = glossTexture.Sample(textureSampler, pin.uv).r;


	if (useDirLight)
	{
		DirectionalLightContribution(material, dirLight, glossSample, pin.normalW, toEyeW, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (usePointLight)
	{
		for (int i = 0; i < 6; i++) {
			PointLightContribution(material, pointLightArray[i], glossSample, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
			totalAmbient += ambient;
			totalDiffuse += diffuse;
			totalSpecular += specular;
		}
	}

	float4 finalColor = (totalAmbient + totalDiffuse) * diffuseTexColor + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;

}