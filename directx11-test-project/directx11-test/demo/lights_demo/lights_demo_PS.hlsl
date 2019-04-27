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
	float2 uv : TEXCOORD0;
	float2 uvMotion : TEXCOORD1;
};



cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	Material material;
	bool usesNormalMapTexture;
	bool usesTwoColorMapTextures;

};



Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D glossTexture : register(t2);
Texture2D movableDiffuseTexture : register(t3);

SamplerState textureSampler: register(s0);

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLight;
	PointLight pointLight0;
	PointLight pointLight1;
	PointLight pointLight2;
	PointLight pointLight3;
	PointLight pointLight4;
	SpotLight spotLight0;
	SpotLight spotLight1;
	SpotLight spotLight2;
	float3 eyePosW;
	float translateValue;
};


cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useSpotLight;
}

cbuffer RarelyChangedTextureCB : register(b3)
{
	bool useColorTextureMap;
	bool useNormalTextureMap;
	bool useGlossTextureMap;
}


float3 BumpNormalW(float2 uv, float3 normalW, float3 tangentW) 
{
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;
	float3 bumpNormalT = 2.0f * normalSample - 1.0f;

	float3x3 TBN = float3x3(tangentW, cross(normalW, tangentW), normalW);
	return mul(bumpNormalT, TBN);
}
void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, float glossiness, out float4 ambient, out float4 diffuse, out float4 specular)
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
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), glossiness);
		//float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular * light.specular;
	}
}



void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, float glossiness, out float4 ambient, out float4 diffuse, out float4 specular)
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
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), glossiness);
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

void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, float glossiness, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// default values
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLightW = light.posW - posW;
	float distance = length(toLightW);

	// early rejection
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
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), glossiness);
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
	float3 bumpedNormalW = float3(0.f, 0.f, 0.f);
	//&& usesNormalMapTexture
	if (useNormalTextureMap && usesNormalMapTexture)
	{
		bumpedNormalW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);
	}
	else
	{
		bumpedNormalW = pin.normalW;
	}
	float glossSample = float(0.0f);
	if (useGlossTextureMap)
	{
		glossSample = glossTexture.Sample(textureSampler, pin.uv).r;
		glossSample = exp2(13.0 *(1.f - glossSample));
	}
	else
	{
		glossSample = material.specular.w;
	}
	

	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;


	if (useDirLight)
	{
		DirectionalLightContribution(material, dirLight, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (usePointLight)
	{
		PointLightContribution(material, pointLight0, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		PointLightContribution(material, pointLight1, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		PointLightContribution(material, pointLight2, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		PointLightContribution(material, pointLight3, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		PointLightContribution(material, pointLight4, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (useSpotLight)
	{
		SpotLightContribution(material, spotLight0, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;


		SpotLightContribution(material, spotLight1, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		SpotLightContribution(material, spotLight2, pin.posW, bumpedNormalW, toEyeW, glossSample, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}
	float4 finalColor=float4(0.f, 0.f, 0.f, 0.f);
	if (useColorTextureMap) 
	{ 
		float4 diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);
		float4 movableDiffuseColor = float4(0.f, 0.f, 0.f, 0.f);
		if (usesTwoColorMapTextures) 
		{
			pin.uvMotion.x+=translateValue;
			movableDiffuseColor = movableDiffuseTexture.Sample(textureSampler, pin.uvMotion);
		}
		finalColor = (movableDiffuseColor + diffuseColor) * (totalAmbient + totalDiffuse) + totalSpecular;
	}		
	else
	{
		finalColor = totalAmbient + totalDiffuse + totalSpecular;
	}
	finalColor.a = totalDiffuse.a;
	return finalColor;
	/*
	
	*/
}
