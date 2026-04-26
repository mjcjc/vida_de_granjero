#include "Common.hlsli"


//정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};


//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorldCb), gmtxView), gmtxProj);
	output.normalW = mul(input.normal, (float3x3)gmtxWorldCb) ;
	output.tangentW = float3(0,0,0);
	output.bitangentW =float3(0,0,0);
	output.positionW = mul(input.position, gmtxWorldCb);
	output.uv = input.uv;

	return(output);
}

//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}