
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
	float3 tangentU : TANGENT;   
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	//float2 uvMotion : TEXCOORD1;


};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	Material material;
};
/*
cbuffer PerObjectTextureCB : register(b5)
{
	bool usesNormalMapTexture;
	bool usesTwoColorMapTextures;
};

cbuffer PerFrameTextureCB : register (b4)
{
	float4x4 texCoordMatrix;
}
*/

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posW = mul(float4(vin.posL, 1.0f), W).xyz;
	vout.normalW = mul(vin.normalL, (float3x3)W_inverseTraspose);
	vout.tangentW = mul(vin.tangentU, (float3x3)W_inverseTraspose);
	vout.tangentW = vout.tangentW - (dot(vout.tangentW, vout.normalW)*vout.normalW);
	vout.posH = mul(float4(vin.posL, 1.0f), WVP);
	vout.uv = vin.uv;
	/*
	if (usesTwoColorMapTextures) 
	{
		vout.uvMotion = mul(float4(vin.uv, 0.0f, 1.0f), texCoordMatrix).xy;
	}
	else
	{
		vout.uvMotion = float2(0.0f, 0.0f);
	}
	*/
	return vout;
}
