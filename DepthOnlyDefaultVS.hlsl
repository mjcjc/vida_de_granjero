#include "Common.hlsli"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};


struct VS_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{

	VS_OUTPUT output;
	float4 positionW = mul(float4(input.position, 1.0f), gmtxWorldCb);
	output.position = mul(positionW, light[nShadowIndex].viewProj);
	return output;
}