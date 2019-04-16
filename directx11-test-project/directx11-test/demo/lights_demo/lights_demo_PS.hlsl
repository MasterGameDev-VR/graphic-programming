struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 normalW : NORMALW;
	float3 posW : POSITIONW;
};

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

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_INV_T;
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

void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ambient += mat.ambient * light.ambient;
	float3 toLightW = -light.dirW;
	float Kd = dot(toLightW, normalW);
	[flatten]
	if (Kd > 0.0f)
	{
		diffuse += Kd * mat.diffuse * light.diffuse;
		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.0f), mat.specular.w);
		specular += Ks * mat.specular * light.specular;
	}
}

void Attenuate(float3 attenuation, float4 color, float distance, out float4 outcolor)
{
	outcolor = color / (attenuation.r + attenuation.g * distance + attenuation.b * ( distance * distance));
}

void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 toLightW = (light.posW - posW);
	float distance = length(toLightW);
	toLightW = normalize(toLightW);
	[flatten]
	if (distance <= light.range)
	{
		ambient += mat.ambient * light.ambient;
		Attenuate(light.attenuation, ambient, distance, ambient);
		float Kd = dot(toLightW, normalW);
		[flatten]
		if (Kd > 0.0f)
		{
			diffuse += Kd * mat.diffuse * light.diffuse;
			Attenuate(light.attenuation, diffuse, distance, diffuse);
			float3 halfVectorW = normalize(toLightW + toEyeW);
			float Ks = pow(max(dot(halfVectorW, normalW), 0.0f), mat.specular.w);
			specular += Ks * mat.specular * light.specular;
			Attenuate(light.attenuation, specular, distance, specular);
		}
	}
}

void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 toLightW = (light.posW - posW);
	float distance = length(toLightW);
	toLightW = normalize(toLightW);
	[flatten]
	if(distance <= light.range)
	{
		float KspotLight = pow(max(dot(-toLightW, light.dirW), 0.0f), light.spot);
		ambient += mat.ambient * light.ambient;
		Attenuate(light.attenuation, ambient, distance, ambient);
		ambient *= KspotLight;
		float Kd = dot(toLightW, normalW);
		[flatten]
		if (Kd > 0.0f)
		{
			diffuse += Kd * mat.diffuse * light.diffuse;
			Attenuate(light.attenuation, diffuse, distance, diffuse);
			diffuse *= KspotLight;
			float3 halfVectorW = normalize(toLightW + toEyeW);
			float Ks = pow(max(dot(halfVectorW, normalW), 0.0f), mat.specular.w);
			specular += Ks * mat.specular * light.specular;
			Attenuate(light.attenuation, specular, distance, specular);
			specular *= KspotLight;
		}
	}
}

float4 main(VertexOut pin) : SV_TARGET
{
	pin.normalW = normalize(pin.normalW);
	float3 toEyeW = normalize(eyePosW - pin.posW);
	float4 totalAmbient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalDiffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 totalSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	DirectionalLightContribution(material, dirLight, pin.normalW, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	PointLightContribution(material, pointLight, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;
	
	SpotLightContribution(material, spotLight, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;

}