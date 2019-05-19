#define POINT_LIGHT_COUNT 4
#define DIRECTIONAL_LIGHT_COUNT 2
#define LIGHTS_THAT_CAST_SHADOWS 1

//maybe it is necessary another input structure 
struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float3 tangentU : TANGENT;   
	float2 uv : TEXCOORD;
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
	DirectionalLight dirLights[DIRECTIONAL_LIGHT_COUNT];
	PointLight pointLights[POINT_LIGHT_COUNT];
	float4x4 LightViewMatrices[LIGHTS_THAT_CAST_SHADOWS];
	float4x4 ProjectionMatrices[LIGHTS_THAT_CAST_SHADOWS];
	float3 eyePosW;
};
/*
struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD0;
	float2 uvMotion : TEXCOORD1;
};
*/




//Texture2D shadowMapTexture : register(t10);
//SamplerState textureSampler : register(s0);

float4 main(VertexIn vin): SV_POSITION
{
	float4 vFromLightPOV = mul(float4(vin.posL, 1.0f), WVP);
	return vFromLightPOV;

}
