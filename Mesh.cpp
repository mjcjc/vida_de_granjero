#include "stdafx.h"
#include "Mesh.h"
#include "GameObject.h"

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}

CMesh::~CMesh()
{
	//ComPtr 스마트 포인터를 썻기에 Release가 잗동호출되긴 한다.
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer.Reset();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer.Reset();

	m_pcpd3dSubSetIndexBuffers.clear();
	m_pcpd3dSubSetIndexUploadBuffers.clear();

}

void CMesh::ReleaseUploadBuffers()
{
	//정점 버퍼를 기본적으로 디폴트 힙으로 생성하기에 Uploadbuffer를 같이 만들어서 연결을 해준다. 그리고 이제 변화가 더 이상 필요없으면 당연히
	//메모리를 아끼기위해서 업로드로 만들어둔건 Release해서 없애줘야 하지.
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer.Reset(); //ComPtr은 Release가 아닌 Reset을 해줘야하며 스마트 포인터니까 사실 Releaase가 필요가 없긴하다
	//m_pd3dVertexUploadBuffer = nullptr; //없애고 나서 재활용하거나 사용할려면 nullptr로 지정해놓는것도 가능하다.
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{

	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL); //토폴로지와 VertexBuffer를 CommandList에 바인딩 중.

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes)) //인덱스 버퍼를 사용하고있는 mesh는 이걸로 Draw 해준다
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
		//인덱스 버퍼가 있으면 인덱스 버퍼를 파이프라인(IA: 입력 조립기)에 연결하고 인덱스를 사용하여 렌더링한다.
	}
	else
	{
		//정점의 갯수, Drawinstanced니까 인스턴싱을  쓰기에 몇개를 그릴건지도 지정(자기 자신 하나만 그릴거니까 1만 넣는다.), Offset 전달 등등
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CMesh::NormalRender(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{

	UpdateShaderVariables(pd3dCommandList);

	OnPreRender(pd3dCommandList, NULL); //토폴로지와 VertexBuffer를 CommandList에 바인딩 중.

	D3D12_PRIMITIVE_TOPOLOGY pimitiveTopolgy = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	pd3dCommandList->IASetPrimitiveTopology(pimitiveTopolgy);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes)) //인덱스 버퍼를 사용하고있는 mesh는 이걸로 Draw 해준다
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
		//인덱스 버퍼가 있으면 인덱스 버퍼를 파이프라인(IA: 입력 조립기)에 연결하고 인덱스를 사용하여 렌더링한다.
	}
	else
	{
		//정점의 갯수, Drawinstanced니까 인스턴싱을  쓰기에 몇개를 그릴건지도 지정(자기 자신 하나만 그릴거니까 1만 넣는다.), Offset 전달 등등
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

//
void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	//Mesh의 프리미티브 유형을 설정한다.
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	//Mesh의 정점 버퍼 뷰를 설정한다.
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	//Mesh의 정점 버퍼 뷰를 렌더링 한다
	
}

//후처리 렌더링이라던가 여러 추가 효과를 만드는데 필요할 것.
void CMesh::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{

}

CTriangleMesh::CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 3;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CDiffusedVertex pVertices[3];
	pVertices[0] = CDiffusedVertex(XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f,
		1.0f));
	pVertices[1] = CDiffusedVertex(XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f,
		1.0f));
	pVertices[2] = CDiffusedVertex(XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(Colors::Blue));
	//ID3D12Resource* rawBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
	//	D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dVertexUploadBuffer.GetAddressOf());


	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dVertexUploadBuffer.GetAddressOf());

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

}

CCubeMeshDiffused::CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice,
		pd3dCommandList)
{
	/*
	* 인덱스 버퍼를 활용하는것과 정점 버퍼만 활용하는거의 차이
	* 1. 기본적으로 모든 오브젝트는 정점 버퍼를 필수로 한다.
	* 2. 하지만 Draw할때 토폴로지 설정에 맞추어 읽어들여 그려야하는데, 이때 정점 버퍼만으로 하면 정육각형 Mesh를 그린다 했을때 36개의 정점 버퍼가 필요하며 이중 최소 3개의 정점은 겹치는 정보를 갖고있지만 서로 토폴로지 형식을 맞추기 위해서 어쩔 수 없이 만들어야 한다.
	* 3. 위 문제를 해결하기 위해서 Index 버퍼를 사용하는데 토폴로지 형식을 위한 중복은 어쩔 수 없이 존재하지만 큰 용량을 갖는 정점 버퍼대신 해당 위치를 가르키는 좌표 즉 인덱스만으로 겹치도록 하면 메모리 효율이 증가할 것.
	* 4. 그래서 인덱스 버퍼를 사용하여 그리는 경우가 존재한다. 아래가 인덱스 버퍼 방식이고 인덱스 버퍼 없이 그릴려면 저 인덱스 겹치는 순서대로 정점을 만들기만 하면 된다.
	*/
	//직육면체는 꼭지점(정점)이 8개이다.
	m_nVertices = 8;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	//정점 버퍼는 직육면체의 꼭지점 8개에 대한 정점 데이터를 가진다.
	CDiffusedVertex pVertices[8];
	pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx, +fy, -fz), RANDOM_COLOR);
	pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx, +fy, -fz), RANDOM_COLOR);
	pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx, +fy, +fz), RANDOM_COLOR);
	pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx, +fy, +fz), RANDOM_COLOR);
	pVertices[4] = CDiffusedVertex(XMFLOAT3(-fx, -fy, -fz), RANDOM_COLOR);
	pVertices[5] = CDiffusedVertex(XMFLOAT3(+fx, -fy, -fz), RANDOM_COLOR);
	pVertices[6] = CDiffusedVertex(XMFLOAT3(+fx, -fy, +fz), RANDOM_COLOR);
	pVertices[7] = CDiffusedVertex(XMFLOAT3(-fx, -fy, +fz), RANDOM_COLOR);
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	/*인덱스 버퍼는 직육면체의 6개의 면(사각형)에 대한 기하 정보를 갖는다. 삼각형 리스트로 직육면체를 표현할 것이
   므로 각 면은 2개의 삼각형을 가지고 각 삼각형은 3개의 정점이 필요하다. 즉, 인덱스 버퍼는 전체 36(=6*2*3)개의 인
   덱스를 가져야 한다.*/
	ComPtr<ID3D12Resource> cpd3dIndexBuffer;
	ComPtr<ID3D12Resource> cpd3dIndexUploadBuffer;
	m_pnSubSetIndices = new int[1];
	m_pnSubSetIndices[0] = 36;
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[1];
	UINT pnIndices[36];
	//ⓐ 앞면(Front) 사각형의 위쪽 삼각형
	pnIndices[0] = 3; pnIndices[1] = 1; pnIndices[2] = 0;
	//ⓑ 앞면(Front) 사각형의 아래쪽 삼각형
	pnIndices[3] = 2; pnIndices[4] = 1; pnIndices[5] = 3;
	//ⓒ 윗면(Top) 사각형의 위쪽 삼각형
	pnIndices[6] = 0; pnIndices[7] = 5; pnIndices[8] = 4;
	//ⓓ 윗면(Top) 사각형의 아래쪽 삼각형
	pnIndices[9] = 1; pnIndices[10] = 5; pnIndices[11] = 0;
	//ⓔ 뒷면(Back) 사각형의 위쪽 삼각형
	pnIndices[12] = 3; pnIndices[13] = 4; pnIndices[14] = 7;
	//ⓕ 뒷면(Back) 사각형의 아래쪽 삼각형
	pnIndices[15] = 0; pnIndices[16] = 4; pnIndices[17] = 3;
	//ⓖ 아래면(Bottom) 사각형의 위쪽 삼각형
	pnIndices[18] = 1; pnIndices[19] = 6; pnIndices[20] = 5;
	//ⓗ 아래면(Bottom) 사각형의 아래쪽 삼각형
	pnIndices[21] = 2; pnIndices[22] = 6; pnIndices[23] = 1;
	//ⓘ 옆면(Left) 사각형의 위쪽 삼각형
	pnIndices[24] = 2; pnIndices[25] = 7; pnIndices[26] = 6;
	//ⓙ 옆면(Left) 사각형의 아래쪽 삼각형
	pnIndices[27] = 3; pnIndices[28] = 7; pnIndices[29] = 2;
	//ⓚ 옆면(Right) 사각형의 위쪽 삼각형
	pnIndices[30] = 6; pnIndices[31] = 4; pnIndices[32] = 5;
	//ⓛ 옆면(Right) 사각형의 아래쪽 삼각형
	pnIndices[33] = 7; pnIndices[34] = 4; pnIndices[35] = 6;
	//인덱스 버퍼를 생성한다.
	cpd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices,
		sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		cpd3dIndexUploadBuffer.GetAddressOf());
	//인덱스 버퍼 뷰를 생성한다.
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = cpd3dIndexBuffer->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT; //Vertex 버퍼와의 차이점 포인트!!
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];
}

CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

CAirplaneMeshDiffused::CAirplaneMeshDiffused(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth,
	XMFLOAT4 xmf4Color) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 24 * 3;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	//위의 그림과 같은 비행기 메쉬를 표현하기 위한 정점 데이터이다.
	CDiffusedVertex pVertices[24 * 3];
	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy -
		y3)) / x1) * x2 + (fy - y3);
	int i = 0;
	//비행기 메쉬의 위쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 아래쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 오른쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/오른쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 왼쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/왼쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}
CAirplaneMeshDiffused::~CAirplaneMeshDiffused()
{
}

CCubeMeshMain::CCubeMeshMain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice,
		pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 인덱스 버퍼 없음 → 6면 × 2삼각형 × 3정점 = 36 정점
	m_nVertices = 36;
	m_nStride = sizeof(CMainVertex);
	m_nOffset = 0;
	m_nSlot = 0;

	const float fx = fWidth * 0.5f;
	const float fy = fHeight * 0.5f;
	const float fz = fDepth * 0.5f;

	auto V = [](float x, float y, float z, XMFLOAT3 n, float u, float v) {
		return CMainVertex(XMFLOAT3(x, y, z), n, XMFLOAT2(u, v));
		};

	CMainVertex pVertices[36];
	int i = 0;

	// 1) Z = -fz  (Back)  N=(0,0,-1)
	pVertices[i++] = V(-fx, +fy, -fz, { 0, 0,-1 }, 0, 0);
	pVertices[i++] = V(+fx, +fy, -fz, { 0, 0,-1 }, 1, 0);
	pVertices[i++] = V(+fx, -fy, -fz, { 0, 0,-1 }, 1, 1);

	pVertices[i++] = V(-fx, +fy, -fz, { 0, 0,-1 }, 0, 0);
	pVertices[i++] = V(+fx, -fy, -fz, { 0, 0,-1 }, 1, 1);
	pVertices[i++] = V(-fx, -fy, -fz, { 0, 0,-1 }, 0, 1);

	// 2) Y = +fy  (Top)   N=(0,1,0)
	pVertices[i++] = V(-fx, +fy, +fz, { 0, 1, 0 }, 0, 0);
	pVertices[i++] = V(+fx, +fy, +fz, { 0, 1, 0 }, 1, 0);
	pVertices[i++] = V(+fx, +fy, -fz, { 0, 1, 0 }, 1, 1);

	pVertices[i++] = V(-fx, +fy, +fz, { 0, 1, 0 }, 0, 0);
	pVertices[i++] = V(+fx, +fy, -fz, { 0, 1, 0 }, 1, 1);
	pVertices[i++] = V(-fx, +fy, -fz, { 0, 1, 0 }, 0, 1);

	// 3) Z = +fz  (Front) N=(0,0,1)
	pVertices[i++] = V(-fx, -fy, +fz, { 0, 0, 1 }, 0, 0);
	pVertices[i++] = V(+fx, -fy, +fz, { 0, 0, 1 }, 1, 0);
	pVertices[i++] = V(+fx, +fy, +fz, { 0, 0, 1 }, 1, 1);

	pVertices[i++] = V(-fx, -fy, +fz, { 0, 0, 1 }, 0, 0);
	pVertices[i++] = V(+fx, +fy, +fz, { 0, 0, 1 }, 1, 1);
	pVertices[i++] = V(-fx, +fy, +fz, { 0, 0, 1 }, 0, 1);

	// 4) Y = -fy  (Bottom) N=(0,-1,0)
	pVertices[i++] = V(-fx, -fy, -fz, { 0,-1, 0 }, 0, 0);
	pVertices[i++] = V(+fx, -fy, -fz, { 0,-1, 0 }, 1, 0);
	pVertices[i++] = V(+fx, -fy, +fz, { 0,-1, 0 }, 1, 1);

	pVertices[i++] = V(-fx, -fy, -fz, { 0,-1, 0 }, 0, 0);
	pVertices[i++] = V(+fx, -fy, +fz, { 0,-1, 0 }, 1, 1);
	pVertices[i++] = V(-fx, -fy, +fz, { 0,-1, 0 }, 0, 1);

	// 5) X = -fx  (Left)  N=(-1,0,0)
	pVertices[i++] = V(-fx, +fy, +fz, { -1, 0, 0 }, 0, 0);
	pVertices[i++] = V(-fx, +fy, -fz, { -1, 0, 0 }, 1, 0);
	pVertices[i++] = V(-fx, -fy, -fz, { -1, 0, 0 }, 1, 1);

	pVertices[i++] = V(-fx, +fy, +fz, { -1, 0, 0 }, 0, 0);
	pVertices[i++] = V(-fx, -fy, -fz, { -1, 0, 0 }, 1, 1);
	pVertices[i++] = V(-fx, -fy, +fz, { -1, 0, 0 }, 0, 1);

	// 6) X = +fx  (Right) N=(1,0,0)
	pVertices[i++] = V(+fx, +fy, -fz, { 1, 0, 0 }, 0, 0);
	pVertices[i++] = V(+fx, +fy, +fz, { 1, 0, 0 }, 1, 0);
	pVertices[i++] = V(+fx, -fy, +fz, { 1, 0, 0 }, 1, 1);

	pVertices[i++] = V(+fx, +fy, -fz, { 1, 0, 0 }, 0, 0);
	pVertices[i++] = V(+fx, -fy, +fz, { 1, 0, 0 }, 1, 1);
	pVertices[i++] = V(+fx, -fy, -fz, { 1, 0, 0 }, 0, 1);
	// VB 생성 (IB 없음)
	m_pd3dVertexBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList, pVertices, sizeof(pVertices),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = sizeof(pVertices);
}

CCubeMeshMain::~CCubeMeshMain()
{
}

//-----------------------------------------------------------------------------------------------------

CSkyBoxMesh::CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList) 
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_cpd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_cpd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

}

CSkyBoxMesh::~CSkyBoxMesh()
{

}

void CSkyBoxMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->IASetVertexBuffers(0,1, &m_d3dPositionBufferView);
}

//-----------------------------------------------------------------------------------------------------

CBoundingCubeMesh::CBoundingCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice,
		pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

	const float fx = fWidth * 0.5f;
	const float fy = fHeight * 0.5f;
	const float fz = fDepth * 0.5f;

	// -----------------------
	// 1. 정점 데이터 (8개)
	// -----------------------
	m_nVertices = 8;
	m_pxmf3Positions = new XMFLOAT3[m_nVertices]
	{
		{-fx, +fy, -fz}, //0
		{+fx, +fy, -fz}, //1
		{+fx, -fy, -fz}, //2
		{-fx, -fy, -fz}, //3

		{-fx, +fy, +fz}, //4
		{+fx, +fy, +fz}, //5
		{+fx, -fy, +fz}, //6
		{-fx, -fy, +fz}, //7
	};

	// VertexBuffer 생성
	m_cpd3dPositionBuffer = CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		m_pxmf3Positions,
		sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;


	// -----------------------
	// 2. 인덱스 데이터 (36개)
	// -----------------------
	int numIndices = 36;
	m_pnSubSetIndices[0] = numIndices;
	m_ppnSubSetIndices[0] = new UINT[numIndices]
	{
		0,1,2,  0,2,3,   // Back
		4,5,1,  4,1,0,   // Top
		3,2,6,  3,6,7,   // Bottom
		7,6,5,  7,5,4,   // Front
		4,0,3,  4,3,7,   // Left
		1,5,6,  1,6,2    // Right
	};

	// IndexBuffer 생성
	ComPtr<ID3D12Resource> ib;
	ComPtr<ID3D12Resource> ibUpload;

	ib = CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		m_ppnSubSetIndices[0],
		sizeof(UINT) * numIndices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&ibUpload);

	m_pcpd3dSubSetIndexBuffers.push_back(ib);
	m_pcpd3dSubSetIndexUploadBuffers.push_back(ibUpload);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = ib->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * numIndices;
}

CBoundingCubeMesh::~CBoundingCubeMesh()
{
}


void CBoundingCubeMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dPositionBufferView);

	//pd3dCommandList->IASetIndexBuffer(,)
}

//-----------------------------------------------------------------------------------------------------

CPlanMesh::CPlanMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	int nSlices, int nStacks,
	float fScaleX, float fScaleY) : CMesh(pd3dDevice, pd3dCommandList)
{

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_nSubMeshes = 1;

	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];


	// 1) 정점 개수 계산

	const int numVertsX = nSlices + 1;
	const int numVertsY = nStacks + 1;
	m_nVertices = numVertsX * numVertsY;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Normals = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	// dx, dy는 [-1 ~ 1] 공간 기준
	float dx = 2.0f / nSlices;
	float dy = 2.0f / nStacks;

	int idx = 0;
	float y = 1.0f;


	// 2) 정점 채우기 

	for (int j = 0; j < numVertsY; j++)
	{
		float x = -1.0f;
		for (int i = 0; i < numVertsX; i++)
		{
			float px = x * fScaleX;
			float py = 0.0f;
			float pz = y * fScaleY;

			m_pxmf3Positions[idx] = XMFLOAT3(px, py, pz);
			m_pxmf3Normals[idx] = XMFLOAT3(0.0f, 1.0f, 0.0f);

			// x,y가 [-1 ~ 1] 범위였으므로 (x+1)/2, (y+1)/2
	/*		m_pxmf2TextureCoords0[idx] = XMFLOAT2(
				(x + 1.0f) * 0.5f,
				(y + 1.0f) * 0.5f
			);*/

			m_pxmf2TextureCoords0[idx] = XMFLOAT2(
				(float)i,
				(float)j
			);
			idx++;
			x += dx;
		}
		y -= dy;
	}


	// 3) 인덱스 생성 

	int numIndices = nSlices * nStacks * 6;
	m_pnSubSetIndices[0] = numIndices;

	m_ppnSubSetIndices[0] = new UINT[numIndices];
	UINT* pIndex = m_ppnSubSetIndices[0];

	idx = 0;
	for (int j = 0; j < nStacks; j++)
	{
		for (int i = 0; i < nSlices; i++)
		{
			int v0 = j * numVertsX + i;
			int v1 = v0 + 1;
			int v2 = v0 + numVertsX;
			int v3 = v2 + 1;

			// 첫 번째 삼각형
			pIndex[idx++] = v0;
			pIndex[idx++] = v1;
			pIndex[idx++] = v2;

			// 두 번째 삼각형
			pIndex[idx++] = v2;
			pIndex[idx++] = v1;
			pIndex[idx++] = v3;
		}
	}


	// 4) VB / IB Upload

	// -- Position VB --
	m_cpd3dPositionBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// -- Normal VB --
	m_cpd3dNormalBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dNormalUploadBuffer);

	m_d3dNormalBufferView.BufferLocation = m_cpd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// -- UV VB --
	m_cpd3dTextureCoord0Buffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_cpd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;


	// 5) IndexBuffer Upload

	ComPtr<ID3D12Resource> ib;
	ComPtr<ID3D12Resource> ibUpload;

	ib = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_ppnSubSetIndices[0],
		sizeof(UINT) * numIndices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&ibUpload);

	m_pcpd3dSubSetIndexBuffers.push_back(ib);
	m_pcpd3dSubSetIndexUploadBuffers.push_back(ibUpload);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = ib->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * numIndices;
}

CPlanMesh::~CPlanMesh()
{

}

void CPlanMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[3] = { m_d3dPositionBufferView ,
		m_d3dNormalBufferView,
		m_d3dTextureCoord0BufferView,
		 };

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, d3dVertexBufferView);
}

void CPlanMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	CMesh::Render(pd3dCommandList, nSubSet);
}

void CPlanMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	//if (m_cpd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	//m_pd3dTextureCoord0UploadBuffer = NULL;

	//if (m_cpd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	//m_pd3dNormalUploadBuffer = NULL;

	//if (m_cpd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	//m_pd3dTangentUploadBuffer = NULL;

	//if (m_cpd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	//m_pd3dBiTangentUploadBuffer = NULL;
}
//-----------------------------------------------------------------------------------------------------

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	//HEIGHTMAP의 DIMENSION
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale; //크기

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++) //이게 보통 이미지를 빠르게 읽는 방식에 접근방법
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			//Texture 좌표상 y축은 반대로 저장해야함으로 아래에서 위방향으로 저장중 y값을 뒤집는중
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}



float CHeightMapImage::GetHeight(float fx, float fz)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);


}

//-----------------------------------------------------------------------------------------------------

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, 
	ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength,
	int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, void* pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CHeightMapImage* pHeightMap = (CHeightMapImage*)pContext;

	const int numVertsX = nBlockWidth;
	const int numVertsZ = nBlockLength;
	m_nVertices = numVertsX * numVertsZ;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf3Normals = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	// Plan과 동일한 중심 보정
	float halfWidth = (numVertsX - 1) * xmf3Scale.x * 0.5f;
	float halfLength = (numVertsZ - 1) * xmf3Scale.z * 0.5f;

	int idx = 0;

	// =========================
	// 1. 정점 생성 (Plan과 동일)
	// =========================
	for (int z = 0; z < nWidth; z++)
	{
		for (int x = 0; x < nLength; x++, idx++)
		{
			int hx = xStart + x  * xmf3Scale.x;
			int hz = zStart + z  * xmf3Scale.z;

			float localX = x * xmf3Scale.x - halfWidth;
			float localZ = z * xmf3Scale.z - halfLength;

			/*float localX = x * xmf3Scale.x - halfWidth * 2;
			float localZ = z * xmf3Scale.z - halfLength * 2;*/

			float height = pHeightMap->GetHeight(hx, hz) * xmf3Scale.y;

			m_pxmf3Positions[idx] = XMFLOAT3(localX, height, localZ);
			m_pxmf3Normals[idx] = pHeightMap->GetHeightMapNormal(hx, hz);

			// Plan과 동일한 UV 방식
			m_pxmf2TextureCoords0[idx] = XMFLOAT2((float)x, (float)z);

			m_pxmf2TextureCoords1[idx] = XMFLOAT2(
				(float)x / (float)(numVertsX - 1),
				(float)z / (float)(numVertsZ - 1)
			);
		}
	}

	// =========================
	// 2. 인덱스 생성 (Plan과 100% 동일)
	// =========================
	int numIndices = (numVertsX - 1) * (numVertsZ - 1) * 6;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[1];
	m_pnSubSetIndices[0] = numIndices;

	m_ppnSubSetIndices = new UINT * [1];
	m_ppnSubSetIndices[0] = new UINT[numIndices];

	UINT* pIndex = m_ppnSubSetIndices[0];
	int i = 0;

	for (int z = 0; z < numVertsZ - 1; z++)
	{
		for (int x = 0; x < numVertsX - 1; x++)
		{
			int v0 = z * numVertsX + x;
			int v1 = v0 + 1;
			int v2 = v0 + numVertsX;
			int v3 = v2 + 1;

		
		// 첫 번째 삼각형
			pIndex[i++] = v0;
			pIndex[i++] = v2;
			pIndex[i++] = v1;

			// 두 번째 삼각형
			pIndex[i++] = v1;
			pIndex[i++] = v2;
			pIndex[i++] = v3;
		}
	}
	// Position VB
	m_cpd3dPositionBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// Normal VB
	m_cpd3dNormalBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dNormalUploadBuffer);

	m_d3dNormalBufferView.BufferLocation = m_cpd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	// UV0 VB
	m_cpd3dTextureCoord0Buffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_cpd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// UV1 VB
	m_cpd3dTextureCoord1Buffer = CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		m_pxmf2TextureCoords1,
		sizeof(XMFLOAT2) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dTextureCoord1UploadBuffer
	);

	m_d3dTextureCoord1BufferView.BufferLocation =
		m_cpd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	// Index Buffer
	ComPtr<ID3D12Resource> ib;
	ComPtr<ID3D12Resource> ibUpload;

	ib = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_ppnSubSetIndices[0],
		sizeof(UINT) * numIndices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&ibUpload);

	m_pcpd3dSubSetIndexBuffers.push_back(ib);
	m_pcpd3dSubSetIndexUploadBuffers.push_back(ibUpload);

	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[1];
	m_pd3dSubSetIndexBufferViews[0].BufferLocation = ib->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * numIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}

void CHeightMapGridMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[4] = { m_d3dPositionBufferView ,
		m_d3dNormalBufferView,
		m_d3dTextureCoord0BufferView,
		m_d3dTextureCoord1BufferView,
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, d3dVertexBufferView);
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	CMesh::Render(pd3dCommandList, nSubSet);
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	//if (m_cpd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	//m_pd3dTextureCoord0UploadBuffer = NULL;

	//if (m_cpd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	//m_pd3dNormalUploadBuffer = NULL;

	//if (m_cpd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	//m_pd3dTangentUploadBuffer = NULL;

	//if (m_cpd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	//m_pd3dBiTangentUploadBuffer = NULL;
}

//-----------------------------------------------------------------------------------------------------
CScreenMesh::CScreenMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{

	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_nSubMeshes = 1;

	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT * [m_nSubMeshes];
	m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];


	//정점 채우기
	m_nVertices = 4;

	m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];

	// NDC 기준 (-1 ~ 1)
	m_pxmf3Positions[0] = XMFLOAT3(-1.0f, 1.0f, 0.0f); // LT
	m_pxmf3Positions[1] = XMFLOAT3(1.0f, 1.0f, 0.0f); // RT
	m_pxmf3Positions[2] = XMFLOAT3(-1.0f, -1.0f, 0.0f); // LB
	m_pxmf3Positions[3] = XMFLOAT3(1.0f, -1.0f, 0.0f); // RB

	// UV (0 ~ 1)
	m_pxmf2TextureCoords0[0] = XMFLOAT2(0.0f, 0.0f);
	m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 0.0f);
	m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
	m_pxmf2TextureCoords0[3] = XMFLOAT2(1.0f, 1.0f);


	// 3) 인덱스 생성 


	m_pnSubSetIndices[0] = 6;
	m_ppnSubSetIndices[0] = new UINT[6];

	UINT* pIndex = m_ppnSubSetIndices[0];

	// Triangle 1
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;

	// Triangle 2
	pIndex[3] = 2;
	pIndex[4] = 1;
	pIndex[5] = 3;


	// 4) VB / IB Upload

	// -- Position VB --
	m_cpd3dPositionBuffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;


	// -- UV VB --
	m_cpd3dTextureCoord0Buffer = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dTextureCoord0UploadBuffer);

	m_d3dTextureCoord0BufferView.BufferLocation = m_cpd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;


	// 5) IndexBuffer Upload

	ComPtr<ID3D12Resource> ib;
	ComPtr<ID3D12Resource> ibUpload;

	ib = CreateBufferResource(
		pd3dDevice, pd3dCommandList,
		m_ppnSubSetIndices[0],
		sizeof(UINT) * 6,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&ibUpload);

	m_pcpd3dSubSetIndexBuffers.push_back(ib);
	m_pcpd3dSubSetIndexUploadBuffers.push_back(ibUpload);

	m_pd3dSubSetIndexBufferViews[0].BufferLocation = ib->GetGPUVirtualAddress();
	m_pd3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_pd3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * 6;
}

CScreenMesh::~CScreenMesh()
{

}

void CScreenMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[2] = { m_d3dPositionBufferView ,
		m_d3dTextureCoord0BufferView,
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, d3dVertexBufferView);
}

void CScreenMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	CMesh::Render(pd3dCommandList, nSubSet);
}

void CScreenMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
}
//-----------------------------------------------------------------------------------------------------
CDefualtMesh::CDefualtMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CDefualtMesh::~CDefualtMesh()
{

}

void CDefualtMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList , void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[5] = { m_d3dPositionBufferView ,
		m_d3dTextureCoord0BufferView, 
		m_d3dNormalBufferView,
		m_d3dTangentBufferView,
		m_d3dBiTangentBufferView};

	pd3dCommandList->IASetVertexBuffers(m_nSlot,5, d3dVertexBufferView);
}

void CDefualtMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList , int nSubSet)
{
	CMesh::Render(pd3dCommandList , nSubSet);
}

void CDefualtMesh::LoadMeshDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

			
		
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);
			
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CDefualtMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_cpd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dPositionUploadBuffer.GetAddressOf());

				m_d3dPositionBufferView.BufferLocation = m_cpd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_cpd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dTextureCoord0UploadBuffer.GetAddressOf());

				m_d3dTextureCoord0BufferView.BufferLocation = m_cpd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_cpd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dTextureCoord1UploadBuffer.GetAddressOf());

				m_d3dTextureCoord1BufferView.BufferLocation = m_cpd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_cpd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dNormalUploadBuffer.GetAddressOf());

				m_d3dNormalBufferView.BufferLocation = m_cpd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_cpd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dTangentUploadBuffer.GetAddressOf());

				m_d3dTangentBufferView.BufferLocation = m_cpd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_cpd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dBiTangentUploadBuffer.GetAddressOf());

				m_d3dBiTangentBufferView.BufferLocation = m_cpd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				m_pcpd3dSubSetIndexBuffers.resize(m_nSubMeshes);
				m_pcpd3dSubSetIndexUploadBuffers.resize(m_nSubMeshes);
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_pcpd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_pcpd3dSubSetIndexUploadBuffers[i].GetAddressOf());

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_pcpd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CDefualtMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	//if (m_cpd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	//m_pd3dTextureCoord0UploadBuffer = NULL;

	//if (m_cpd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	//m_pd3dNormalUploadBuffer = NULL;

	//if (m_cpd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	//m_pd3dTangentUploadBuffer = NULL;

	//if (m_cpd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	//m_pd3dBiTangentUploadBuffer = NULL;
}


//------------------------------------------------------------------------------------------

CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CDefualtMesh(pd3dDevice, pd3dCommandList)
{

}

CSkinnedMesh::~CSkinnedMesh()
{

}

void CSkinnedMesh::LoadSkinInfoDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				//m_ppSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					//m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
		
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
	
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
			
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
				m_cpd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
				m_cpd3dcbBindPoseBoneOffsets->Map(0, NULL, (void**)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_cpd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dBoneIndexUploadBuffer.GetAddressOf());

				m_d3dBoneIndexBufferView.BufferLocation = m_cpd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_cpd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_cpd3dBoneWeightUploadBuffer.GetAddressOf());

				m_d3dBoneWeightBufferView.BufferLocation = m_cpd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[7] = { 
		m_d3dPositionBufferView ,
		m_d3dTextureCoord0BufferView,
		m_d3dNormalBufferView,
		m_d3dTangentBufferView,
		m_d3dBiTangentBufferView,
		m_d3dBoneIndexBufferView,
		m_d3dBoneWeightBufferView,
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, d3dVertexBufferView);
}

void CSkinnedMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	CMesh::Render(pd3dCommandList, nSubSet);
}

void CSkinnedMesh::PrepareSkinning(CGameObject* pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);
	}
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CDefualtMesh::ReleaseUploadBuffers();

	//if (m_cpd3dBoneIndexUploadBuffer) m_cpd3dBoneIndexUploadBuffer->Release();
	//m_pd3dBoneIndexUploadBuffer = NULL;

	//if (m_cpd3dBoneWeightUploadBuffer) m_cpd3dBoneWeightUploadBuffer->Release();
	//m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_cpd3dcbBindPoseBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dBoneOffsetsGpuVirtualAddress = m_cpd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(11,d3dBoneOffsetsGpuVirtualAddress);
	}

	if (m_cpd3dcbSkinningBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_cpd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(12, d3dcbBoneTransformsGpuVirtualAddress);
	
		for (int i = 0; i < m_nSkinningBones; i++) //
		{
			//
			XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[i], 
				XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[i]->m_xmf4x4World)));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

ParticleMesh::ParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, int nCount, UINT nTypeFlag) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = nCount;

	// POINT LIST
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	// Position Buffer (더미)
	m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	for (int i = 0; i < m_nVertices; ++i)
	{
		m_pxmf3Positions[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	m_cpd3dPositionBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		m_pxmf3Positions,
		sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dPositionUploadBuffer
	);

	m_d3dPositionBufferView.BufferLocation =
		m_cpd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes =
		sizeof(XMFLOAT3) * m_nVertices;


	// ParticleSet Buffer
	// x = lifeTime
	// y = speed
	// z = spawnInterval
	XMFLOAT3* pxmf3ParticleSet = new XMFLOAT3[m_nVertices];

	for (int i = 0; i < m_nVertices; ++i)
	{
		float lifeTime = 0.0f;
		float speed = 0.0f;
		float interval = 0.0f;

		// 타입별 기본 범위
		if (nTypeFlag & PF_RAIN)
		{
			lifeTime = RandomFloat(1.5f, 2.5f);
			speed = RandomFloat(8.0f, 12.0f);
			interval = RandomFloat(0.1f, 0.3f);
		}
		else if (nTypeFlag & PF_SNOW)
		{
			lifeTime = RandomFloat(2.0f, 4.0f);
			speed = RandomFloat(1.0f, 2.0f);
			interval = RandomFloat(0.3f, 0.6f);
		}
		else if (nTypeFlag & PF_DUST)
		{
			lifeTime = RandomFloat(5.0f, 10.0f);
			speed = RandomFloat(0.2f, 0.5f);
			interval = RandomFloat(0.5f, 1.0f);
		}
		else if (nTypeFlag & PF_PUNCH)
		{
			lifeTime = RandomFloat(0.1f, 0.5f);
			//lifeTime = 0.5f;
			speed = RandomFloat(2.0f, 4.0f);
			interval = RandomFloat(0.1f, 0.2f);
			//interval = 0.0f;
		}

		pxmf3ParticleSet[i] = XMFLOAT3(lifeTime, speed, interval);
	}

	m_cpd3dParticleSetBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		pxmf3ParticleSet,
		sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dParticleSetUploadBuffer
	);

	m_d3dParticleSetBufferView.BufferLocation =
		m_cpd3dParticleSetBuffer->GetGPUVirtualAddress();
	m_d3dParticleSetBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dParticleSetBufferView.SizeInBytes =
		sizeof(XMFLOAT3) * m_nVertices;

	XMFLOAT2* pxmf2ParticleSize = new XMFLOAT2[m_nVertices];

	for (int i = 0; i < m_nVertices; ++i)
	{
		float sizeX = 0.1f;
		float sizeY = 0.1f;

		if (nTypeFlag & PF_RAIN)
		{
			// 비: 가늘고 길게
			sizeX = RandomFloat(0.01f, 0.03f);   // 가로 매우 얇게
			sizeY = RandomFloat(0.15f, 0.35f);   // 세로 길게
		}
		else if (nTypeFlag & PF_SNOW)
		{
			// 눈: 둥글거나 약간 불규칙
			sizeX = RandomFloat(0.06f, 0.14f);
			sizeY = sizeX;
		}
		else if (nTypeFlag & PF_DUST)
		{
			// 황사/연기: 큼 + 약간 납작
			sizeX = RandomFloat(0.25f, 0.5f);
			sizeY = sizeX;
		}

		else if (nTypeFlag & PF_PUNCH)
		{

			sizeX = RandomFloat(0.01f, 0.05f);
			sizeY = sizeX;
		}

		pxmf2ParticleSize[i] = XMFLOAT2(sizeX, sizeY);
	}

	m_cpd3dParticleSizeBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		pxmf2ParticleSize,
		sizeof(XMFLOAT2) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		&m_cpd3dParticleSizeUploadBuffer
	);

	m_d3dParticleSizeBufferView.BufferLocation =
		m_cpd3dParticleSizeBuffer->GetGPUVirtualAddress();
	m_d3dParticleSizeBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dParticleSizeBufferView.SizeInBytes =
		sizeof(XMFLOAT2) * m_nVertices;

	delete[] pxmf2ParticleSize;
	delete[] pxmf3ParticleSet;

}

ParticleMesh::~ParticleMesh()
{

}

void ParticleMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);


	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView[3] = { m_d3dPositionBufferView ,
	m_d3dParticleSetBufferView, m_d3dParticleSizeBufferView
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 3, d3dVertexBufferView);

}
