#include "Common.hlsli"

//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return(input.color);
}