#define FRAME_BUFFER_WIDTH 640.0f
#define FRAME_BUFFER_HEIGHT 480.0f


//float4 VSMain(uint nVertexID : SV_VertexID) : SV_POSITION
//{
//     float4 output = (float4)0;
//    if (nVertexID == 0) output = float4(-1.0f, +1.0f, 0.0f, 1.0f);
//    else if (nVertexID == 1) output = float4(+1.0f, +1.0f, 0.0f, 1.0f);
//    else if (nVertexID == 2) output = float4(+1.0f, -1.0f, 0.0f, 1.0f);
//    else if (nVertexID == 3) output = float4(-1.0f, +1.0f, 0.0f, 1.0f);
//    else if (nVertexID == 4) output = float4(+1.0f, -1.0f, 0.0f, 1.0f);
//    else if (nVertexID == 5) output = float4(-1.0f, -1.0f, 0.0f, 1.0f);
//    
//	return output;
//}
//
//float4 PSMain(float4 input : SV_Position) : SV_Target
//{
//     float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//     cColor.r = input.x / FRAME_BUFFER_WIDTH;
//    return cColor;
//
//}

//정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};
//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다.
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

/*
* 패킹 오프셋의 c0, c1, c2에 단위 크기는 float4 즉 16바이트이다.
* 그래서 matrix = float4x4임으로 cbCameraInfo의 packoffset(c4)가 나오는 이유는 c0 기준 1,2,3까지 float4가 4개 차지 하고 있음으로 그 다음인 c4부터 시작하는 것.
* 항상 16바이트 묶음으로 읽는게 원칙임으로 float, float2, float3 값을 넘겨야할때 중간 중간 패딩 값으로 float3, float2, float를 넣어서 끊어 읽을 수 있도록 해줘야
* 속도적인 측면이나 안정측면에서 좋다.
*/
cbuffer cbGameObjectInfo : register(b0)
{
	matrix gmtxWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProj : packoffset(c4);
};

VS_OUTPUT VSDiffused(VS_INPUT input)
{
	VS_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProj);
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	return(input.color);
}