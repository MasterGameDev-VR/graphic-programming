struct VertexOut {
	float4 posH : SV_POSITION;
	float4 prevposH : PREV_POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
	pin.posH.xyz /= pin.posH.w;
	pin.prevposH.xyz /= pin.prevposH.w;
	float4 vel = pin.posH - pin.prevposH;
	return float4(vel.x, vel.y, 0.0f, 0.0f);
}