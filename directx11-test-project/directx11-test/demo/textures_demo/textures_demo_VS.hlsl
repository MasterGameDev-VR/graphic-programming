
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
	float3 tangentL : TANGENT;
	float2 uv : TEXCOORD;
};


struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 uv : TEXCOORD;
	float2 uvMov : TEXCOORDMOV;
};


cbuffer PerObjectCB : register(b0)
{
	float4x4 W;
	float4x4 W_inverseTraspose;
	float4x4 WVP;
	float4x4 TexcoordMatrix;
	Material material;
	float4x4 MovingTexcoordMatrix;
	bool HasMovingTexture;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.posW = mul(float4(vin.posL, 1.0f), W).xyz;
	vout.normalW = mul(vin.normalL, (float3x3)W_inverseTraspose);
	vout.posH = mul(float4(vin.posL, 1.0f), WVP);
	vout.tangentW = vin.tangentL;

	vout.uv = mul(float4(vin.uv, 0.0f, 1.0f), TexcoordMatrix).xy;
	
	vout.uvMov = float2(0.0f, 0.0f);
	if (HasMovingTexture) {
		vout.uvMov = mul(float4(vin.uv, 0.0f, 1.0f), MovingTexcoordMatrix).xy;
	}

	return vout;
}