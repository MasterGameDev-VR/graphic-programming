struct VertexOut {
	float4 posH : SV_POSITION;
	float4 prevposH : PREV_POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
	pin.posH.xyz /= pin.posH.w;
	pin.prevposH.xyz /= pin.prevposH.w;

	return pin.prevposH - pin.posH;
}