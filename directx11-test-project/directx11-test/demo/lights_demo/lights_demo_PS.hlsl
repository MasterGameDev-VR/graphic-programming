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
};



cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useSpotLight;
}


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
	ambient  *= falloff;
	diffuse  *= attenuationFactor * falloff;
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
	ambient  *= spot;
	diffuse  *= spot * attenuationFactor;
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

	
	if (useDirLight)
	{
		DirectionalLightContribution(material, dirLight, pin.normalW, toEyeW, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (usePointLight)
	{
		PointLightContribution(material, pointLight, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}
	
	if (useSpotLight)
	{
		SpotLightContribution(material, spotLight, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}
	

	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;

}