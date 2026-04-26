#include "Common.hlsli"


struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct GS_OUTPUT
{
	float4 position : SV_POSITION;
};

GS_OUTPUT main(VS_INPUT input)
{
	GS_OUTPUT output;
	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorldCb).xyz;
	output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProj);
	return output;
}