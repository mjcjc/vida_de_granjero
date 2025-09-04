#pragma once


class CVertex
{

protected:
	//정점의 위치 벡터 ==> 정점이 존재하기 위해선 가장 기본으로 위치를 갖고있어야한다
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {}
};

// 정점에 가장 기본은 위치이지만 그것말고도 색상 그자체를 갖고도 있다. 나중에 Texture를 넣는다고 한다면 이 클래스를 쓰지 않아도 될 것이다.
class CDiffusedVertex : public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse;

public:
	CDiffusedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); }
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4DiffuseColor) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4DiffuseColor; }
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4DiffuseColor) { m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4DiffuseColor; }
	~CDiffusedVertex() {}
};

/*
*  게임상의 Mesh 모델은 정점들의 집합으로 이루어져있다.
*  그래서 위에 Vertex라는 클래스를 만든 것이다.
*/
class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommnadList);
	virtual ~CMesh(); //Mesh를 상속받아서 사용할 다양한 모델mesh이 존재할테니까 가상화를 써준다.
private:
	int m_nReferences = 0; //Direct12는 기본적으로 레퍼런스로 관리를하고 있기에 레퍼런스 카운터를 넣어준것. ==> 하나의 Mesh를 공유하는 오브젝트가 다양하기에 레퍼런스로 관리하는게 또한 용이하다.
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void ReleaseUploadBuffers();
protected:
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer = NULL; //정점 버퍼
	ComPtr<ID3D12Resource> m_pd3dVertexUploadBuffer = NULL; // 정점 버퍼에 데이터를 저장하고 변경해주고 해줄 업로드 버퍼

	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView; //버퍼들을 실제로 GPU에 연결하고 명령어를 내릴때 연결할 실 주소이자 역할을 지정하는 부분이라고 보면된다.

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; //이거는 Draw할때 정점에 정보를 어떤기준으로 읽을지를 하는것이며 이건 삼각형기준으로 읽고 정점의 인덱스 정보를 이어서 취급한다는 의미이다.
	UINT m_nSlot = 0;  //
	UINT m_nVertices = 0; //
	UINT m_nStride = 0; //
	UINT m_nOffset = 0; //
protected:
	ComPtr<ID3D12Resource> m_pd3dIndexBuffer = NULL;
	ComPtr<ID3D12Resource> m_pd3dIndexUploadBuffer = NULL;
	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다. 인덱스 버퍼는 정점
   버퍼(배열)에 대한 인덱스를 가진다.*/
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
	UINT m_nIndices = 0;
	//인덱스 버퍼에 포함되는 인덱스의 개수이다. 
	UINT m_nStartIndex = 0;
	//인덱스 버퍼에서 메쉬를 그리기 위해 사용되는 시작 인덱스이다. 
	int m_nBaseVertex = 0;
	//인덱스 버퍼의 인덱스에 더해질 인덱스이다.
public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList); //이 함수에서 실제 Draw 콜이 발생할 것이다.  ==> 인스턴싱을 사용한다고 하면 모든 Mesh들 마다 드로우 콜을 호출하진 않을것이다. 하지만 기본적으로 Mesh마다 호출이 되는걸로 알고있는게 좋다.
};

class CTriangleMesh : public CMesh
{
public:
	CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTriangleMesh() {}
};

class CCubeMeshDiffused : public CMesh
{
public:
	//직육면체의 가로, 세로, 깊이의 길이를 지정하여 직육면체 메쉬를 생성한다.
	CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshDiffused();
};

class CAirplaneMeshDiffused : public CMesh //플레이어의 Mesh
{
public:
	CAirplaneMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f,
		XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CAirplaneMeshDiffused();
};