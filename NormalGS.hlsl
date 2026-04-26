#include "Common.hlsli"

/*
* 아무리 GS가 들어가서 VS -> PS를 통과하지않고 VS -> GS -> PS를 통과해서 OUTPUT이 바뀔 수 있다해도
* PS에 Input은 VS에 Output에 시맨틱과 일치하거나 혹은 없는걸 추가하면 안된다. 있는걸 빼는건 가능하다
*/
struct GS_OUTPUT
{
	float4 pos : SV_POSITION;

};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;

};

[maxvertexcount(2)] //정점을 한개만 추가할꺼니까 2개로 설정
void main( point VS_OUTPUT input[1] , inout LineStream< GS_OUTPUT > output
)
{
	GS_OUTPUT p0, p1;

	float4 newColor = float4(1.0,1,1,1);


	p0.pos = input[0].position;
	//p0.color = newColor;
	newColor = float4(1, 0, 0, 1);


	float3 worldPos = input[0].positionW;
	float3 worldNormal = normalize(input[0].normalW);

	float3 endPosW = worldPos + worldNormal * 1.0;

	float4 endPosH = mul(mul(float4(endPosW, 1.0f), gmtxView), gmtxProj);

	p1.pos = endPosH;
	//p1.color = newColor;

	output.Append(p0);
	output.Append(p1);
	output.RestartStrip(); //라인별로 스트립 끊는다
}