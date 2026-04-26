#pragma once
class CGameObject;


/////////////////////////////////////////////////////////////////////////////////////
/// 어떤 정점 성질을 갖고있는지 판단하기 위한 비트 플래그.
#define VERTEXT_POSITION				0x0001
#define VERTEXT_COLOR					0x0002
#define VERTEXT_NORMAL					0x0004
#define VERTEXT_TANGENT					0x0008
#define VERTEXT_TEXTURE_COORD0			0x0010
#define VERTEXT_TEXTURE_COORD1			0x0020

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)

/////////////////////////////////////////////////////////////////////////////////////////
//
#define SKINNED_ANIMATION_BONES		256 // 스키닝 가능한 최대 본 개수

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

class CTextureVertex : public CVertex
{
protected:
	XMFLOAT2 m_xmf2UV;
	CTextureVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf2UV = XMFLOAT2(0.0f, 0.0f); }
	CTextureVertex(float x, float y, float z, XMFLOAT2 xmf2TexCoord) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf2UV = xmf2TexCoord; }
	CTextureVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2TexCoord) { m_xmf3Position = xmf3Position; m_xmf2UV = xmf2TexCoord; }
	~CTextureVertex() {}
};

//position, normal, texcoord ==> Texture 맵핑과 조명 계산을 위한 정점
class CMainVertex : public CVertex
{
protected:
	XMFLOAT3 m_xmf3Normal;
	XMFLOAT2 m_xmf2texcoord;
public:
	CMainVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2texcoord = XMFLOAT2(0.0f, 0.0f);
	}

	CMainVertex(float x, float y, float z, XMFLOAT3
		xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT2 xmf2texcoord = XMFLOAT2(0.0f, 0.0f))
	{
		m_xmf3Position = XMFLOAT3(x, y, z);
		m_xmf3Normal = xmf3Normal;
		m_xmf2texcoord = xmf2texcoord;
	}

	CMainVertex(XMFLOAT3 xmf3Position, XMFLOAT3
		xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT2 xmf2texcoord = XMFLOAT2(0.0f, 0.0f))
	{
		m_xmf3Position = xmf3Position;
		m_xmf3Normal = xmf3Normal;
		m_xmf2texcoord = xmf2texcoord;
	}

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
public: 
	char							m_pstrMeshName[64] = { 0 };
protected:
	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; //이거는 Draw할때 정점에 정보를 어떤기준으로 읽을지를 하는것이며 이건 삼각형기준으로 읽고 정점의 인덱스 정보를 이어서 취급한다는 의미이다.

	//vertex를 하나의 리소스를 관리하는 경우
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer = NULL; //정점 버퍼
	ComPtr<ID3D12Resource> m_pd3dVertexUploadBuffer = NULL; // 정점 버퍼에 데이터를 저장하고 변경해주고 해줄 업로드 버퍼
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView; //버퍼들을 실제로 GPU에 연결하고 명령어를 내릴때 연결할 실 주소이자 역할을 지정하는 부분이라고 보면된다.


	//vertex 버퍼를 여러 리소스로 나누워 관리하는 경우

	UINT m_nType = 0x00;


	UINT m_nSlot = 0;  //
	UINT m_nVertices = 0; //
	UINT m_nStride = 0; //
	UINT m_nOffset = 0; //

	//AABB
	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	//이건 나중에 절두체 컬링에 활용할 예정.
	BoundingBox m_BoundingBox;

protected:

	//ComPtr<ID3D12Resource> m_pd3dIndexBuffer = NULL;
	//ComPtr<ID3D12Resource> m_pd3dIndexUploadBuffer = NULL;
	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다. 인덱스 버퍼는 정점
   버퍼(배열)에 대한 인덱스를 가진다.*/
	//D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	int		m_nSubMeshes = 0;
	
	int* m_pnSubSetIndices = NULL;

	UINT** m_ppnSubSetIndices = NULL;

	std::vector<ComPtr<ID3D12Resource>> m_pcpd3dSubSetIndexBuffers;
	std::vector<ComPtr<ID3D12Resource>> m_pcpd3dSubSetIndexUploadBuffers;
	/*인덱스 버퍼(인덱스의 배열)와 인덱스 버퍼를 위한 업로드 버퍼에 대한 인터페이스 포인터이다. 인덱스 버퍼는 정점
   버퍼(배열)에 대한 인덱스를 가진다.*/
	D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews;


	//UINT m_nIndices = 0;
	//인덱스 버퍼에 포함되는 인덱스의 개수이다. 
	//UINT m_nStartIndex = 0;
	//인덱스 버퍼에서 메쉬를 그리기 위해 사용되는 시작 인덱스이다. 
	//int m_nBaseVertex = 0;
	//인덱스 버퍼의 인덱스에 더해질 인덱스이다.

public:
	UINT GetType() { return(m_nType); }

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext );
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet); //이 함수에서 실제 Draw 콜이 발생할 것이다.  ==> 인스턴싱을 사용한다고 하면 모든 Mesh들 마다 드로우 콜을 호출하진 않을것이다. 하지만 기본적으로 Mesh마다 호출이 되는걸로 알고있는게 좋다.
	virtual void OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);

	virtual void NormalRender(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {};
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

class CCubeMeshMain : public CMesh
{
public:
	//직육면체의 가로, 세로, 깊이의 길이를 지정하여 직육면체 메쉬를 생성한다.
	CCubeMeshMain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshMain();
};


class CAirplaneMeshDiffused : public CMesh //플레이어의 Mesh
{
public:
	CAirplaneMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f,
		XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CAirplaneMeshDiffused();
};

class CSkyBoxMesh : public CMesh
{
public:

	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);

	XMFLOAT3* m_pxmf3Positions = NULL;

	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;
};

class CBoundingCubeMesh : public CMesh
{
public:
	//직육면체의 가로, 세로, 깊이의 길이를 지정하여 직육면체 메쉬를 생성한다.
	CBoundingCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CBoundingCubeMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);

	XMFLOAT3* m_pxmf3Positions = NULL;

	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;
};

class CPlanMesh : public CMesh
{

public:
	CPlanMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		int nSlices, int nStacks,
		float fScaleX, float fScaleY);
	virtual ~CPlanMesh();

protected:
	XMFLOAT3* m_pxmf3Positions = NULL;
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;

	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2* m_pxmf2TextureCoords1 = NULL;
	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;

	// TexCoord0
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord0BufferView;

	// TexCoord1
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord1BufferView;

	// Normal
	ComPtr<ID3D12Resource>         m_cpd3dNormalBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dNormalBufferView;

public:
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void ReleaseUploadBuffers();

};

class CHeightMapImage
{
private:
	//높이 맵 이미지 픽셀(8-비트)들의 이차원 배열이다. 각 픽셀은 0~255의 값을 갖는다.
	BYTE
		* m_pHeightMapPixels;
	//높이 맵 이미지의 가로와 세로 크기이다.
	int		m_nWidth;
	int	m_nLength;

	//높이 맵 이미지를 실제로 몇 배 확대하여 사용할 것인가를 나타내는 스케일 벡터이다.
	XMFLOAT3
		m_xmf3Scale;
public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);
	//높이 맵 이미지에서 (x, z) 위치의 픽셀 값에 기반한 지형의 높이를 반환한다.
	float GetHeight(float x, float z);
	//높이 맵 이미지에서 (x, z) 위치의 법선 벡터를 반환한다.
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};
class CHeightMapGridMesh : public CMesh
{
protected:
	//격자의 크기(가로: x-방향, 세로: z-방향)이다.
	int 	m_nWidth;
	int 	m_nLength;
	
	/*격자의 스케일(가로: x-방향, 세로: z-방향, 높이: y-방향) 벡터이다. 실제 격자 메쉬의 각 정점의 x-좌표, y-좌표,
	z-좌표는 스케일 벡터의 x-좌표, y-좌표, z-좌표로 곱한 값을 갖는다. 즉, 실제 격자의 x-축 방향의 간격은 1이 아니
	라 스케일 벡터의 x-좌표가 된다. 이렇게 하면 작은 격자(적은 정점)를 사용하더라도 큰 크기의 격자(지형)를 생성할
	수 있다.*/
	XMFLOAT3 	m_xmf3Scale;

	XMFLOAT3* m_pxmf3Positions = NULL;
	XMFLOAT4* m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;

	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2* m_pxmf2TextureCoords1 = NULL;
	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;

	// TexCoord0
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord0BufferView;

	// TexCoord1
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord1BufferView;

	// Normal
	ComPtr<ID3D12Resource>         m_cpd3dNormalBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dNormalBufferView;
public:

	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale =
		XMFLOAT3(1.0f, 1.0f, 1.0f), void
		* pContext = NULL);
	virtual ~CHeightMapGridMesh();
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }
	//격자의 좌표가 (x, z)일 때 교점(정점)의 높이를 반환하는 함수이다.
	virtual float OnGetHeight(int x, int z, void* pContext);

	//격자의 좌표가 (x, z)일 때 교점(정점)의 색상을 반환하는 함수이다.
	//virtual XMFLOAT4 OnGetColor(int x, int z, void* pContext);

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void ReleaseUploadBuffers();
};

class CScreenMesh : public CMesh
{

public:
	CScreenMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CScreenMesh();

protected:
	XMFLOAT3* m_pxmf3Positions = NULL;
	XMFLOAT4* m_pxmf4Colors = NULL;

	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;

	// TexCoord0
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord0BufferView;



public:
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void ReleaseUploadBuffers();

};


//외부 모델을 Import 해와서 init 해주는 Mesh
class CDefualtMesh : public CMesh
{

public:
	CDefualtMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CDefualtMesh();

protected:
	XMFLOAT3* m_pxmf3Positions = NULL;
	XMFLOAT4 *m_pxmf4Colors = NULL;
	XMFLOAT3* m_pxmf3Normals = NULL;
	XMFLOAT3* m_pxmf3Tangents = NULL;
	XMFLOAT3* m_pxmf3BiTangents = NULL;

	XMFLOAT2* m_pxmf2TextureCoords0 = NULL;
	XMFLOAT2* m_pxmf2TextureCoords1 = NULL;
	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;

	// TexCoord0
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord0BufferView;

	// TexCoord1
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1Buffer;
	ComPtr<ID3D12Resource>         m_cpd3dTextureCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTextureCoord1BufferView;

	// Normal
	ComPtr<ID3D12Resource>         m_cpd3dNormalBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dNormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dNormalBufferView;

	// Tangent
	ComPtr<ID3D12Resource>         m_cpd3dTangentBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dTangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dTangentBufferView;

	// BiTangent
	ComPtr<ID3D12Resource>         m_cpd3dBiTangentBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dBiTangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dBiTangentBufferView;

public:

	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	//Mesh를 읽기만 하기 위한 부분
	void LoadMeshDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual void ReleaseUploadBuffers();
};

class CSkinnedMesh : public CDefualtMesh
{
	public:
		CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
		virtual ~CSkinnedMesh();
protected:
	// Bone Indices
	ComPtr<ID3D12Resource>   m_cpd3dBoneIndexBuffer;
	ComPtr<ID3D12Resource>   m_cpd3dBoneIndexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dBoneIndexBufferView;

	// Bone Weights
	ComPtr<ID3D12Resource>   m_cpd3dBoneWeightBuffer;
	ComPtr<ID3D12Resource>   m_cpd3dBoneWeightUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3dBoneWeightBufferView;

protected:
	int m_nBonesPerVertex = 4; //정점은 4개의 Bone의 영향을 받는다
	XMINT4* m_pxmn4BoneIndices = NULL; //4개의 
	XMFLOAT4* m_pxmf4BoneWeights = NULL;

public:
	int								m_nSkinningBones = 0;

	char							(*m_ppstrSkinningBoneNames)[64]; //[m_nSkinningBones]
	CGameObject** m_ppSkinningBoneFrameCaches = NULL; //[m_nSkinningBones]

	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL; //[m_nSkinningBones], Transposed

	ComPtr<ID3D12Resource> m_cpd3dcbBindPoseBoneOffsets = NULL; //[m_nSkinningBones]
	XMFLOAT4X4* m_pcbxmf4x4MappedBindPoseBoneOffsets = NULL; //[m_nSkinningBones]

	ComPtr<ID3D12Resource> m_cpd3dcbSkinningBoneTransforms = NULL; //[m_nSkinningBones], Pointer Only
	XMFLOAT4X4* m_pcbxmf4x4MappedSkinningBoneTransforms = NULL; //[m_nSkinningBones]

public:
	//스키닝을 하기전 Caches Bone 설정
	void PrepareSkinning(CGameObject* pModelRootObject);
	//파일로 부터 스키닝 정보를 읽어온다.
	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	
	//파일로 부터 스키닝 정보를 읽어온다 ==> 더미 버전 Texture 갯수를
	//분석학 위한 더미
	void LoadSkinInfoDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList , int nSubSet);

	virtual void ReleaseUploadBuffers();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
};

/*
* TODO: 
 Render 변경된 것 들 추가 , 재질 설정 , Shader 설정
캐릭터를 위한 GameObject 하나 추가할 것. 
*/
//////////////////////////////////
class ParticleMesh : public CMesh
{
public:

	ParticleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nCount, UINT nTypeFlag);
	virtual ~ParticleMesh();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);

	XMFLOAT3* m_pxmf3Positions = NULL;

	// Position
	ComPtr<ID3D12Resource>         m_cpd3dPositionBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dPositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dPositionBufferView;

	//LifeTime , Speed , Interval
	ComPtr<ID3D12Resource>         m_cpd3dParticleSetBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dParticleSetUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dParticleSetBufferView;

	//size
	ComPtr<ID3D12Resource>         m_cpd3dParticleSizeBuffer;
	ComPtr<ID3D12Resource>         m_cpd3dParticleSizeUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW       m_d3dParticleSizeBufferView;

};
