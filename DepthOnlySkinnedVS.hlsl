#include "Common.hlsli"

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
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{

	VS_OUTPUT output;
	float4x4 mtxVertexToBoneWorld = (float4x4)0.0f;

	int4 indices = input.indices;
	float4 weights = input.weights;

	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
	{
		// 4개의 Bone의 가중치만큼 영향을 받은 월드 좌표.
		mtxVertexToBoneWorld += weights[i] *
			mul(gpmtxBoneOffsets[indices[i]], gpmtxBoneTransforms[indices[i]]);
	}

	float3 positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;

	output.position = mul(float4(positionW, 1.0f), light[nShadowIndex].viewProj);

	return(output);
}