cbuffer PerObjectCB : register(b0)
{
	float4x4 WVP;
};


float4 main(float3 posL : POSITION) : SV_POSITION
{
	return mul(float4(posL, 1.0f), WVP);
}
