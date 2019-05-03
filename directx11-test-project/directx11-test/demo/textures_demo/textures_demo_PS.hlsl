struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 options; //0 has texture //1 texture tiles //2 displace //3 animated
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
	float4 posH		: SV_POSITION;
	float3 posW		: POSITION;
	float3 normalW	: NORMAL;
	float3 tangentW	: TANGENT;
	float2 uv		: TEXCOORD;
	float2 uvAnim		: TEXCOORD1;
	
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
	DirectionalLight dirLight;
	PointLight pointLight[5];
	SpotLight spotLight;
	float3 eyePosW;
};

cbuffer RarelyChangedCB : register(b2)
{
	bool useDirLight;
	bool usePointLight;
	bool useSpotLight;
}


void DirectionalLightContribution(Material mat, DirectionalLight light, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular, float glossSample)
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
		float Ks;
		float3 halfVectorW = normalize(toLightW + toEyeW);
		if (glossSample >= 0.0f) {
			float exponent = exp2(11.0f * (1.f - glossSample));
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		}
		else {
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		}

		specular += Ks * mat.specular * light.specular;
	}
}


void PointLightContribution(Material mat, PointLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular, float glossSample)
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
		float Ks;
		float3 halfVectorW = normalize(toLightW + toEyeW);
		if (glossSample >= 0.0f) {
			float exponent = exp2(14.0f*(1.f - glossSample));
			//float Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		}
		else 
		{
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		}
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

void SpotLightContribution(Material mat, SpotLight light, float3 posW, float3 normalW, float3 toEyeW, out float4 ambient, out float4 diffuse, out float4 specular, float glossSample)
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
		float Ks;
		float3 halfVectorW = normalize(toLightW + toEyeW);

		if (glossSample >= 0.0f) {
			float exponent = exp2(18.0f*(1.f - glossSample));
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), exponent);
		}
		else {
			Ks = pow(max(dot(halfVectorW, normalW), 0.f), mat.specular.w);
		}
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

Texture2D diffuseTexture: register(t0);
Texture2D normalTexture: register(t1);
Texture2D glossTexture: register(t2);
Texture2D animatedTexture: register(t3);

SamplerState textureSampler: register(s0);

float3 BumpNormalW(float2 uv, float3 normalW, float3 tangentW) {
	float3 normalSample = normalTexture.Sample(textureSampler, uv).rgb;
	
	float3 bumpNormalT = 2.f  *normalSample - 1.f;

	float3x3 TBN = float3x3 (tangentW,cross(normalW,tangentW), normalW);

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
	float4 diffuseColor;
	float3 normalBW;
	float glossSample;

	pin.tangentW = pin.tangentW - (dot(pin.tangentW,pin.normalW)*pin.normalW);
	pin.tangentW = normalize(pin.tangentW);

	
	 if (material.options.x == 1 || material.options.x == 3) {
		pin.uv *= material.options.y;

		
		normalBW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);
		glossSample = glossTexture.Sample(textureSampler, pin.uv).r;
		diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);

		if (material.options.x == 3) {
			float4 animatedColor = animatedTexture.Sample(textureSampler, pin.uvAnim);
			if (animatedColor.x>0.015 && animatedColor.y > 0.015 && animatedColor.z > 0.015)
				diffuseColor = animatedColor;
		}
		

	 } else if (material.options.x == 2) {
		 pin.uv *= material.options.y;

		 diffuseColor = diffuseTexture.Sample(textureSampler, pin.uv);
		 normalBW = BumpNormalW(pin.uv, pin.normalW, pin.tangentW);
		 glossSample = -1;
	 } else
	 {
		 //reset textures
		 glossSample = -1;
		 normalBW = pin.normalW;
		 diffuseColor = 1;
	 }


	if (useDirLight)
	{
	//	DirectionalLightContribution(material, dirLight, pin.normalW, toEyeW, ambient, diffuse, specular);
		DirectionalLightContribution(material, dirLight, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (usePointLight)
	{
		//PointLightContribution(material, pointLight[0], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		PointLightContribution(material, pointLight[0], pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		//PointLightContribution(material, pointLight[1], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		PointLightContribution(material, pointLight[1], pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		//PointLightContribution(material, pointLight[2], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		PointLightContribution(material, pointLight[2], pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		//PointLightContribution(material, pointLight[3], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		PointLightContribution(material, pointLight[3], pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;

		//PointLightContribution(material, pointLight[4], pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		PointLightContribution(material, pointLight[4], pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}

	if (useSpotLight)
	{
		//SpotLightContribution(material, spotLight, pin.posW, pin.normalW, toEyeW, ambient, diffuse, specular);
		SpotLightContribution(material, spotLight, pin.posW, normalBW, toEyeW, ambient, diffuse, specular, glossSample);
		totalAmbient += ambient;
		totalDiffuse += diffuse;
		totalSpecular += specular;
	}


	float4 finalColor = diffuseColor *( totalAmbient + totalDiffuse) + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;
	//return diffuseColor;
}