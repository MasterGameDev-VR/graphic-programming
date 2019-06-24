struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
	Material material;
};

float4 main(float4 posH : SV_POSITION) : SV_TARGET
{
	float4 totalAmbient = material.ambient;
	float4 totalDiffuse = material.diffuse;
	float4 totalSpecular = material.specular;

	float4 finalColor = (totalAmbient + totalDiffuse) + totalSpecular;
	finalColor.a = totalDiffuse.a;

	return finalColor;
}