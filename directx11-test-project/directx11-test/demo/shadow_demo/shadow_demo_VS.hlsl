
struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float4 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};


struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float4 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	float4 shadowPosH : SHADOWPOS;
	float4 projectorPosH : PROJECTORPOS;
};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	float4x4 WVPT_shadowMap;
	float4x4 WVPT_projector;
	Material material;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posW = mul(float4(vin.posL, 1.0f), W).xyz;
	vout.normalW = mul(vin.normalL, (float3x3)W_inverseTraspose);
	float3 tgW = mul(vin.tangentL.xyz, (float3x3)W);
	vout.tangentW = float4(tgW, vin.tangentL.w);
	// vout.tangentW = mul(vin.tangentL, (float3x3)W);

	vout.posH = mul(float4(vin.posL, 1.0f), WVP);
	vout.uv = mul(float4(vin.uv, 0.f, 1.f), TexcoordMatrix).xy;
	vout.shadowPosH = mul(float4(vin.posL, 1.0f), WVPT_shadowMap);
	vout.projectorPosH = mul(float4(vin.posL, 1.0f), WVPT_projector);

	return vout;
}
