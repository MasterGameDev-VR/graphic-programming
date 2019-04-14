
struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

<<<<<<< HEAD

cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_Inv_Transp;
	float4x4 WVP;
	Material material;
};


=======
>>>>>>> master
struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
<<<<<<< HEAD
	float3 tangentU : TANGENT;
	float2 uv : TEXCOORD;
=======
>>>>>>> master
};

struct VertexOut
{
	float4 posH : SV_POSITION;
<<<<<<< HEAD
	float4 posW : POSITION;
	float4 normalW : NORMAL;
};

void main( VertexIn vin, out VertexOut vout  ) 
{
	float4 posLout = float4(vin.posL, 1.0f);
	float4 normalWout = float4(vin.normalL, 1.0f);
	vout.posW = mul(posLout, W);
	vout.posH = mul(posLout, WVP);
	vout.normalW = mul(normalWout, W_Inv_Transp);
	//vout.normalW = mul(vout.normalW, W_Inv_Transp);
=======
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


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posW = mul(float4(vin.posL, 1.0f), W).xyz;
	vout.normalW = mul(vin.normalL, (float3x3)W_inverseTraspose);
	vout.posH = mul(float4(vin.posL, 1.0f), WVP);

	return vout;
>>>>>>> master
}