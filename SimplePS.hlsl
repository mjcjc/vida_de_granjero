struct GS_OUTPUT
{
	float4 position : SV_POSITION;
};

float4 main(GS_OUTPUT input) : SV_TARGET
{
	return float4(1,1,1,1);
}