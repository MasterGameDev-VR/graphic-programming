
struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_Inv_Transp;
	float4x4 WVP;
	Material material;
};


struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float3 tangentU : TANGENT;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float4 normalW : NORMAL;
};

void main( VertexIn vin, out VertexOut vout  ) 
{
	float4 posLout = float4(vin.posL, 1.0f);
	float4 normalWout = float4(vin.normalL, 1.0f);
	vout.posH = mul(posLout, WVP);
	vout.normalW = mul(normalWout, W_Inv_Transp);
	//vout.normalW = mul(vout.normalW, W_Inv_Transp);
}