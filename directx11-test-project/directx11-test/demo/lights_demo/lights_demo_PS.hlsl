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
	float4x4 W_inverseTranspose;
	float4x4 WVP;
	Material material;
};

cbuffer PerFrameCB : register(b1)
{
	DirectionalLight directionalLight;
	PointLight pointLight;
	SpotLight spotLight;
	float3 eyePosW;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
};



void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW,
								out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	ambient += mat.ambient * light.ambient;

	float3 toLightW = -light.dirW;
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f) 
	{
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular * light.specular;
	}
}



void PointLightContribution(Material mat, PointLight light, float3 position, float3 normalW, float3 toEyeW,
						out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 distanceVector = light.posW - position;
	float lightDistance = length(distanceVector);

	[flatten]
	if (lightDistance < light.range) {

		float3 toLightW = normalize(distanceVector);
		float Kd = dot(toLightW, normalW);

		float attenuation = light.attenuation.x + light.attenuation.y * lightDistance + light.attenuation.z * pow(lightDistance, 2);

		ambient += (mat.ambient * light.ambient) / attenuation;
		ambient *= (light.range - lightDistance);

		[flatten]
		if (Kd > 0.f) {
			diffuse += (Kd * mat.diffuse * light.diffuse) / attenuation;
			diffuse *= (light.range - lightDistance);

			float3 halfVectorW = normalize(toLightW + toEyeW);
			float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
			specular += (Ks * mat.specular * light.specular) / attenuation;
			specular *= (light.range - lightDistance);
		}
	}
}



void SpotLightContribution(Material mat, SpotLight light, float3 position, float3 normalW, float3 toEyeW,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 distanceVector = light.posW - position;
	float lightDistance = length(distanceVector);
	   
	[flatten]
	if (lightDistance < light.range) {

		float3 toLightW = normalize(distanceVector);

		float Kspot = pow(max(dot(-toLightW, light.dirW), 0), light.spot);

		[flatten]
		if (Kspot > 0) {
			float Kd = dot(toLightW, normalW);

			float attenuation = light.attenuation.x + light.attenuation.y * lightDistance + light.attenuation.z * pow(lightDistance, 2);

			ambient += mat.ambient * light.ambient / attenuation * Kspot;
			ambient *= (light.range - lightDistance);

			[flatten]
			if (Kd > 0.f) {
				diffuse += Kd * mat.diffuse * light.diffuse / attenuation * Kspot;
				diffuse *= (light.range - lightDistance);

				float3 halfVectorW = normalize(toLightW + toEyeW);
				float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
				specular += Ks * mat.specular * light.specular / attenuation * Kspot;
				specular *= (light.range - lightDistance);
			}
		}
	}
}


float4 main(VertexOut pin) : SV_Target
{
	pin.normalW = normalize(pin.normalW);
	float3 toEyeW = normalize(eyePosW - pin.posW);

	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	DirectionalLightContribution(material, directionalLight, pin.normalW, toEyeW, ambient, diffuse, specular);
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
