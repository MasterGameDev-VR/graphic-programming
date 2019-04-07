struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

//CONSTANT BUFFERS TO USE IN THE PIXEL SHADER!!
cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_Inv_Transp;
	float4x4 WVP;
	Material material;
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
cbuffer PerFrameCB :register(b1) 
{
	DirectionalLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;
	float3 eyePosW;
};
//-----------------------


struct VertexOut
{
	float4 posH : SV_POSITION;
	float4 normalW : NORMAL;
	
};

void DirectionalLightContribution(Material mat, DirectionalLight light, float4 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ambient += mat.ambient *light.ambient;
	//-------------------
	float3 toLightW = -light.dirW; //already normalized
	float Kd = dot(toLightW, float3(normalW.xyz));

	//----------
	[flatten]
	if (Kd > 0.0f)
	{
		diffuse += Kd * light.diffuse *mat.diffuse;

		float3 halfVectorW = normalize(toLightW + toEyeW);
		float Ks = pow(max(dot(halfVectorW, float3(normalW.x, normalW.y, normalW.z)), 0.0f), mat.specular.w);
		specular += Ks * mat.specular* light.specular;

	};
};

void PointLightContribution(Material mat, PointLight light, float4 normalW, float3 toEyeW, float3 posW, out float4 ambient, out float4 diffuse, out float4 specular)

{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLightW = light.posW - posW;

	[flatten]
	if (length(toLightW) < light.range)
	{
		//calcolo distanza dalla luce: solo se la distanza è minore del range
		float distance = length(toLightW);
		toLightW = normalize(toLightW);
		//componente ambientale solo entro un certo range
		ambient += mat.ambient *light.ambient;
		float attenuation = light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * pow(distance, 2);
		[flatten]
		if (attenuation >= 1.0f) {
			ambient /= attenuation;
		}
		
		float Kd = dot(toLightW, float3(normalW.x, normalW.y, normalW.z));
		[flatten]
		if (Kd > 0.0f)
		{
			diffuse += Kd * light.diffuse *mat.diffuse;
			
			float3 halfVectorW = normalize(toLightW + toEyeW);
			float Ks = pow(max(dot(halfVectorW, float3(normalW.x, normalW.y, normalW.z)), 0.0f), mat.specular.w);
			specular += Ks *mat.specular* light.specular;
			if (attenuation >= 1.0f) {
				diffuse /= attenuation;
				specular /= attenuation;
			}
		}
	}
}

void SpotLightContribution(Material mat,SpotLight light, float4 normalW, float3 toEyeW, float3 posW, out float4 ambient, out float4 diffuse, out float4 specular)

{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 toLightW = light.posW - posW;

	[flatten]
	if (length(toLightW) < light.range)
	{
		//calcolo distanza dalla luce: solo se la distanza è minore del range
		float distance = length(toLightW);
		toLightW = normalize(toLightW);

		//coefficiente di fallOff
		float Kfs = max(dot(-toLightW, light.dirW), 0.0f);

		//componente ambientale solo entro un certo range
		ambient += mat.ambient *light.ambient;
		float attenuation = light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * pow(distance, 2);
		[flatten]
		if (attenuation >= 1.0f) {
			ambient /= attenuation;
		}
		[flatten]
		if (Kfs < light.spot)
		{

			float Kd = dot(toLightW, float3(normalW.x, normalW.y, normalW.z));
			[flatten]
			if (Kd > 0.0f)
			{
				diffuse += Kfs * Kd * light.diffuse *mat.diffuse;
				float3 halfVectorW = normalize(toLightW + toEyeW);
				float Ks = pow(max(dot(halfVectorW, float3(normalW.x, normalW.y, normalW.z)), 0.0f), mat.specular.w);
				specular += Kfs * Ks *mat.specular* light.specular;
				[flatten]
				if (attenuation >= 1.0f) {
					diffuse /= attenuation;
					specular /= attenuation;
				}

			}
		}
	}
}


float4 main(VertexOut pin) : SV_TARGET
{
	float3 posW = pin.posH.xyz;
	float3 toEyeW = normalize(eyePosW - posW);
	pin.normalW = normalize(pin.normalW);



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

	PointLightContribution(material, pointLight, pin.normalW, toEyeW, posW, ambient, diffuse, specular);
	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	SpotLightContribution(material, spotLight, pin.normalW, toEyeW, posW, ambient, diffuse, specular);

	totalAmbient += ambient;
	totalDiffuse += diffuse;
	totalSpecular += specular;

	float4 finalColor = totalAmbient + totalDiffuse + totalSpecular;
	finalColor.a = totalDiffuse.a;
	return finalColor;

}
