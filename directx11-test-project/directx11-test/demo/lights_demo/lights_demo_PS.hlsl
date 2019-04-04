
struct DirectionalLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dirW;
};

struct PointLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 attenuation;
};

struct SpotLight {
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 posW;
	float range;
	float3 dirW;
	float spot;
	float3 attenuation;
};

struct Material {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	Material material;
};

cbuffer PerFrameCB : register(b1) {
	DirectionalLight dirLight;
	PointLight pLight;
	SpotLight sLight;
	float3 eyePosW;
}



struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normal : NORMAL;
};

void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular) {

	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	ambient += mat.ambient * light.ambient;

	float3 toLightW = -light.dirW;
	float Kd = dot(toLightW, normalW);

	[flatten]
	if (Kd > 0.f) {
		diffuse += Kd * mat.diffuse * light.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		specular += Ks * mat.specular * light.specular;
	}
}

void PointLightContribution(Material mat, PointLight light, float3 position, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular) {

	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightRaw = light.posW - position;
	float distance = length(toLightRaw);

	[flatten]
	if (distance < light.range) {

		float3 toLightW = normalize(toLightRaw);
		float Kd = dot(toLightW, normalW);

		float attenuation = light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance;

		ambient += mat.ambient * light.ambient / attenuation;

		[flatten]
		if (Kd > 0.f) {
			diffuse += Kd * mat.diffuse * light.diffuse / attenuation;;

			float3 halfVectorW = normalize(toLightW + toEyeW);
			float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
			specular += Ks * mat.specular * light.specular / attenuation;;
		}
	}
}

void SpotLightContribution(Material mat, SpotLight light, float3 position, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular) {

	ambient = float4(0.f, 0.f, 0.f, 0.f);
	diffuse = float4(0.f, 0.f, 0.f, 0.f);
	specular = float4(0.f, 0.f, 0.f, 0.f);

	float3 toLightRaw = light.posW - position;
	float distance = length(toLightRaw);

	

	[flatten]
	if (distance < light.range) {

		float3 toLightW = normalize(toLightRaw);
		
		float Kspot = pow(max(dot(-toLightW, light.dirW), 0), light.spot);

		[flatten]
		if (Kspot > 0) {
			float Kd = dot(toLightW, normalW);

			float attenuation = light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * distance;

			ambient += mat.ambient * light.ambient / attenuation * Kspot;

			[flatten]
			if (Kd > 0.f) {
				diffuse += Kd * mat.diffuse * light.diffuse / attenuation * Kspot;

				float3 halfVectorW = normalize(toLightW + toEyeW);
				float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
				specular += Ks * mat.specular * light.specular / attenuation * Kspot;
			}
		}
	}
}

float4 main(VertexOut pin) : SV_Target
{
	pin.normal = normalize(pin.normal);

	float3 toEyeW = normalize(eyePosW - pin.posW);

	float4 totalAmbient = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalDiffuse = float4(0.f, 0.f, 0.f, 0.f);
	float4 totalSpecular = float4(0.f, 0.f, 0.f, 0.f);

	float4 ambient;
	float4 diffuse;
	float4 specular;

	DirectionalLightContribution(material, dirLight, pin.normal, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	PointLightContribution(material, pLight, pin.posW, pin.normal, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	SpotLightContribution(material, sLight, pin.posW, pin.normal, toEyeW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;
}
