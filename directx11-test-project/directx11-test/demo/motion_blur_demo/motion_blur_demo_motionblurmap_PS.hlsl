struct VertexOut {
	float4 posH : SV_POSITION;
	float4 pos : CURRENT_POSITION;
	float4 prevpos : PREV_POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
	pin.pos.xyz /= pin.pos.w;
	pin.prevpos.xyz /= pin.prevpos.w;
	float4 vel = pin.pos - pin.prevpos;
	return float4(vel.x, vel.y, 0.0f, 0.0f);
}