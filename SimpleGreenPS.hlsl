struct VS_OUTPUT
{
	float3 positionL : POSITION;
	float4 position : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return float4(0,1,0,1);
}