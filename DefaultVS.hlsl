#include "Common.hlsli"


//정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
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



	//output.positionW = mul(float4(input.position, 1.0f), gmtxWorldCb).xyz;

	output.positionW = mul(float4(input.position, 1.0f), gmtxWorldCb).xyz;
	//output.positionW = input.position;
	output.normalW = mul(input.normal, (float3x3)gmtxWorldCb).xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxWorldCb).xyz;
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxWorldCb).xyz;

	//output.normalW.z *= -1;
	//output.tangentW.z *= -1;
	//output.bitangentW.z *= -1;

	output.uv = input.uv;

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProj);

	//output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorldCb), gmtxView),gmtxProj);

	return(output);
}

//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}