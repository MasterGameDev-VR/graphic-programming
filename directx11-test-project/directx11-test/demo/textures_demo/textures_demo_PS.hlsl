#define SPOTNUMBER 5

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
	float4x4 TextcoordMatrix;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLight;
	PointLight pointLight[SPOTNUMBER];
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useSpotLight;
}

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);

SamplerState textureSampler : register(s0);


void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
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
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular * light.specular;
	}
}
void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
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

	// custom "gentle" falloff
	float falloff = 1.f - (distance / light.range);

	// attenuation
	float attenuationFactor = 1.0f / dot(light.attenuation, float3(1.0f, distance, distance*distance));
	ambient *= falloff;
	diffuse *= attenuationFactor * falloff;
	specular *= attenuationFactor * falloff;

}


float3 BumpNormalW(float2 uv, float3 normalW, float3 tangentW) 
{
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;

	float3 bumpNormalT = 2.f * normalSample - 1.f;

	float3x3 TBN = float3x3(tangentW, cross(normalW, tangentW), normalW);

	return mul(bumpNormalT, TBN);
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

	pin.tangentW = pin.tangentW - (dot(pin.tangentW, pin.normalW) * pin.normalW);
	pin.tangentW = normalize(pin.tangentW);

	pin.normalW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);

	if (useDirLight)
	{
		DirectionalLightContribution(material, dirLight, pin.normalW, toEyeW, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (usePointLight)
	{
		for (int i = 0; i < SPOTNUMBER; i++) {
			PointLightContribution(material, pointLight[i], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
			totalAmbient += ambient;
			totalDiffuse += diffuse;
			totalSpecular += specular;
		}
	}  

	float4 diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);

	float4 finalColor = diffuseColor * (totalAmbient + totalDiffuse) + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;

}