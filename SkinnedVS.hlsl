#include "Common.hlsli"


//정점 셰이더의 입력을 위한 구조체를 선언한다.
struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	int4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
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
	/*
	* indices : 해당 정점에 영향을 주는 Bone의 index
	* weights : 해당 정점에 영향을 주는 Bone들의 얼만큼 영향을 줄지에 대한 가중치 현재 4개에 영향만 받는다.
	* gpmtxBoneTransforms : Bone의 Transform 
	* gpmtxBoneOffsets : Root Bone 기준으로 되돌리는 역행렬 ==> 이걸 vertex의 position에다가 곱하면
	* 해당 Root 본 기준에서 해당 정점에 현재 영향을 주는 Bone에 어느 위치 쯤 있는지를 구할 수 있고
	* 그걸 구한 뒤에 BoneTransform을 곱해야 Bone의 움직임을 정점에 부여할 수 있다.
	*/

	//gmtxWorldCb

	VS_OUTPUT output;
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;

	int4 indices = input.indices;
	float4 weights = input.weights;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i ++)
	{
		// 4개의 Bone의 가중치만큼 영향을 받은 월드 좌표.
		mtxVertexToBoneWorld  += weights[i]* 
			mul(gpmtxBoneOffsets[indices[i]], gpmtxBoneTransforms[indices[i]]);
	}

	output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
	output.normalW = mul(float4(input.normal, 1.0f), (float3x3)mtxVertexToBoneWorld).xyz;
	output.tangentW = mul(float4(input.tangent, 1.0f), (float3x3)mtxVertexToBoneWorld).xyz;
	output.bitangentW = mul(float4(input.bitangent, 1.0f), (float3x3)mtxVertexToBoneWorld).xyz;

	
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProj);

	//output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorldCb), gmtxView),gmtxProj);

	output.uv = input.uv;

	return(output);
}

//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}