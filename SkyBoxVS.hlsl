#include "Common.hlsli"


struct VS_INPUT
{
	float3 position : POSITION;
};

struct VS_OUTPUT
{
	float3 positionL : POSITION;
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0), gmtxWorldCb), gmtxView), gmtxProj);
	output.positionL = input.position;

	return output;
}