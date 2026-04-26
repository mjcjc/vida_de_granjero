#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Animation.h"
#include "ServerPacketHandler.h"

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CShader;
class COBBShader;
//class CAnimationCallbackHandler;
//class CAnimationClips;
//class CAnimationClip;
//class CAnimationController;

enum class EObjectTag //오브젝트 논리상 구별을 위한 것 ==> 이건 아마 충돌 처리 로직이나 그런데 쓰일듯
{
	None = 0,
	Player = 1,
	Item = 2,
	MissionObject = 3,
	Environment = 4,
	UI = 5,
	Crop = 6,
	Floor = 7, //건물 바닥
	Wall = 8,
	Ceiling = 9,
	Resource = 10,
};

//해당 오브젝트가 어떤 모델을 갖고있는지 같은거나 혹은 특정 오브젝트가 가져야하는 설정을 
//판단하기 위해 그리고 서버에서도 전달 받고 관리할려면 중요할듯.
enum class EObjectModelType 
{
	None = 0,

	// Characters (100 단위)
	WoodMan = 100,
	WorkMan = 101,

	// Corn (200 단위)
	Corn01 = 200,
	Corn02 = 201,
	Corn03 = 202,
	Corn04 = 203,
	CornSeed = 210,

	// Potato (300 단위)
	Potato01 = 300,
	Potato02 = 301,
	Potato03 = 302,
	Potato04 = 303,
	PotatoSeed = 310,

	// Pumpkin (400 단위)
	Pumpkin01 = 400,
	Pumpkin02 = 401,
	Pumpkin03 = 402,
	Pumpkin04 = 403,
	PumpkinSeed = 410,

	// Tomato (500 단위)
	Tomato01 = 500,
	Tomato02 = 501,
	Tomato03 = 502,
	Tomato04 = 503,
	TomatoSeed = 510,

	// Others (600 단위)
	LogPlanter = 600,
	Barn = 601,
	Rock01 = 610,
	Rock02 = 611,
	Wall = 620,
	Ceiling = 621,
	Floor = 622,
	Tree01 = 630,
	Tree02 = 631,
	Grass01 = 640,
	Grass02 = 641,
};


//게임 객체의 정보를 Shader 에 넘겨주기 위한 구조체(상수 버퍼)이다. 
struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
};

//struct CB_GAMEOBJECT_MATERIAL
//{
//	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
//
//	UINT m_nType = 0x00;
//};

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	virtual ~CTexture();
private:
	int m_nReferences = 0 ; // Mesh와 같이 Texture 또한 하나의  Texture를 여러개의
	//오브젝트가 참조하거나 사용할 수 있음으로 References 갯수가 필요하다.

	UINT m_nTextureType; //Texture의 Type을 지정해줘야 한다 Type에는
	/*
	* 1. Texture1D
	* 2. Texture1DArray
	* 3. Texture2D
	* 4. Texture2DArray
	* 5. Texture3D
	* 6. Texture3DArray
	* 7. TextureCube
	* 8. TextureCubeArray
	*/

	int m_nTextures = 0; //Texture 갯수
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	ComPtr<ID3D12Resource>* m_ppd3dTextures = NULL;
	ComPtr<ID3D12Resource>* m_ppd3dTextureUploadBuffers;

	ComPtr<ID3D12Resource> texsss;
	
	//ID3D12Resource* t = texsss.Get()[1];

	UINT* m_pnResourceTypes = NULL; //

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
	int* m_pnBufferElements = NULL;

	int m_nRootParameters = 0;
	int* m_pnRootParameterIndices = NULL; //Compute에서 사용한다면 루트 파라미터가 Graphics Pipline하곤 다를것.
	int* m_pnComputeRootParameterIndices = NULL; //UAV를 위한것

	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dUavGpuDescriptorHandles = NULL;

	int m_nSamplers = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

public:

	ComPtr<ID3D12Resource> CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, 
		D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex);
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateComputeShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, bool bIsSrvActive);
	void ReleaseShaderVariables();

	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	void LoadTextureFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetComputeRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);
	void SetUavGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);



	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	_TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
	// 텍스쳐 리소스를 가져오는 함수와 디스크립터 힙에 주소를 가져오는 함수
	//Texture의 종류에 따라 index가 필요할 것이다.
	ComPtr<ID3D12Resource> GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGpuDescriptorHandle(int nIndex) { return(m_pd3dUavGpuDescriptorHandles[nIndex]); }
	int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();

};

/*
* TODO:
* Material의 PBR을 위한 재질 구조체 또한 추가로 넣어줄 예정.
* Shader를 material이 갖는 구조로 개선을 한다.
* Shader에 역할을 정말 PSO 상태를 Shader 종류에 따라 분류만 하도록 변경할 예정이다.
* 현재 Shader는 Object와 PSO 그리고 PS, VS 설정을 전부 갖고있다이걸 GraphicsCommon
* 으로 권한을 옮길 것이다.
*/

//오브젝트 TextureMAP
#define MATERIAL_ALBEDO_MAP        0x01    // 0000 0001
#define MATERIAL_NORMAL_MAP        0x02    // 0000 0010
#define MATERIAL_ROUGHNESS_MAP     0x04    // 0000 0100
#define MATERIAL_METALNESS_MAP     0x08    // 0000 1000
#define MATERIAL_AO_MAP            0x10    // 0001 0000
#define MATERIAL_SPECULARF0_MAP    0x20    // 0010 0000
#define MATERIAL_EMISSION_MAP      0x40    // 0100 0000
#define MATERIAL_SMOOTHNESS_MAP 0x80 // 1000 0000

class CMaterial
{
	
public:
	CMaterial();
	CMaterial(int nTextures); //m_ppTextures를 초기화해주기 위한 생성자
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//기본 색상 의미한다

	int 							m_nTextures = 0;
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	CTexture** m_ppTextures = NULL; // 계층 구조로 이루어진 오브젝트들은
	//각자의 파트마다 Texture를 따로 가질 수 있음으로 이중 포인터 활용.

	//이 부분은 아마 빼야하지만, 이전에 계층 구조 없는 일반 오브젝트들이 이 구조로
	//이루어진 상태라 지금 당장은 안뺄 예정. 
	//TODELETE : 
	CTexture* m_pTexture = NULL;
	CShader* m_pShader = NULL;

	void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetTexture(CTexture* pTexture);
	void SetShader(CShader* pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();

public:
	//
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//Glossiness => Roughness로 사용
	float							m_fRoughness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00; 
	//어느 종류의 Texture를 사용할지를 넘겨줄 플래그 값.



	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void RemoveMaterialType(UINT nType)
	{
		m_nType &= ~nType;  // 플래그 OFF
	}
	
public:
	//Material이 Shader를 무조건 갖고있는 구조 나도 이 구조로 변경해서 사용할려 했음.
	static CShader* m_pStandardShader; //애니메이션이 설정되지 않은 일반 Material
	static CShader* m_pSkinnedAnimationShader; //애니메이션이 존재하는 경우

	//저 2개의 Shader를 초기화해주는 함수 ==> 현재는 그대로 사용하고
	//나중에 기능을 좀 더 추가하거나 세분화 해야할듯
	static void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }

	//파일로 부터 Texture 이름을 받아오는 함수라고 보면 된다
	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//더미
	int LoadTextureDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter, _TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader);
};

//로드된 모델 자산을 한 군데에 모아 관리해주는 컨테이너. 이거 하나로
//1. 읽어온 오브젝트의 Root 정보에 접근하여 새로운 루트 아래 자식으로 추가 ==> 보통의 게임 엔진에서 빈 오브젝트를 만들고 집어넣는것과 같은 동작을 쉽게 처리 가능
//2. Animation은 보통 상용 엔진에서 Animatnio, AnimationController, Animator 이렇게 나누는데
//Animation 설정을 AnimationConroller에 넘기는 역할으 분명히 하기 좋다.
//3. 불 필요한 메모리 사용없이 정보만 빼서 접근하기 용이하다.
class CLoadedModelInfo
{
public:
	CLoadedModelInfo() { }
	~CLoadedModelInfo();

	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

	CAnimationClips* m_pAnimationClips = NULL;

	EObjectModelType m_ObjectType = EObjectModelType::None;

public:
	void PrepareSkinning();
};


//오브젝트 고유 ID
typedef uint32_t ObjectID;

class CGameObject
{
public:
	CGameObject();
	CGameObject(int nMaterials); //Materials를 직접 할당하는 오브젝트들에 대해서 필요한
	//생성자 ==> Import해오는 오브젝트들은 m_nMaterials를 따로 읽어오지만 
	//내가 직접 정의해서 추가하는 친구들은 없다
	virtual ~CGameObject();

	//오브젝트 풀링을 위해서 추가한 초기화 함수
	virtual void Init() {};

	//레퍼런스 카운팅으로 관리할꺼니까 여기도 References 변수를 만들고 AddRef와 Relese를 만들어준다. Direct12에서 기본적으로 인터페이스들을 레퍼런스 카운팅으로 관리하도록 되어 있기 때문에 동일시 해주는 것이다.
private:
	int m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

	XMFLOAT4X4 m_xmf4x4World; //월드 좌표계상의 자신의 위치 행렬
	//부모 위치 ==> 이걸 알아야 계층 애니메이션이 가능.
	XMFLOAT4X4						m_xmf4x4ToParent;

	int         m_nAnimation = 0;               // 현재 애니메이션 번호
	float       m_fAnimationTime = 0.0f;        // 현재 애니메이션 시간

	//0 이면 기본, 1 이면 파랑색상, 2이면 빨간 색상 
	UINT m_nIsBuildingObject = 0;

public:
	//충돌 박스 
	BoundingOrientedBox m_obbBox;
	
	//충돌 처리를 킬지 안킬지 ==> 이건 여러 계층 구조 이루고있는 GameObject안에 저 
	//m_obbBox가 여러개 할당되니까 특정 계층에 오브젝트만 활성화하거나 하는 관리를 위해서
	bool m_bIsBoxCollision = false; 
	bool m_bIsBoxTriggerCollision = false; //충돌 처리 연산 없이 상호작용 하기만 하는 용도로 활성화

	//OBB박스 설정 ==> 이 기본 설정은 충돌체크를 할 박스들에 한해서 작동하도록 한다.
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};

	//obbmesh
	CMesh* m_pOBBMesh = NULL;

	//모든 오브젝트의 위치 기준이 발바닥이랑 가장 밑단이여서 offset으로
	//각 오브젝트 크기 정의에 맞춰서 옮겨줘야할 필요가 있다.
	XMFLOAT3 m_xmf3OBBOffset = XMFLOAT3(0,0,0);

	COBBShader* m_pOBBShader = NULL;

private:
	bool m_bIsActive = false; //오브젝트 동작 상태 
	EObjectTag m_Tag = EObjectTag::None; //오브젝트 논리 태그
	ObjectID m_nID = 0; // 서버/클라 공용 ID
	EObjectModelType m_objType; //정확히 어떤 오브젝트인지 구분

public:

	inline void SetID(ObjectID id) { m_nID = id; }
	inline void SetTag(EObjectTag tag) { m_Tag = tag; }
	inline void SetType(EObjectModelType type) { m_objType = type; }
	inline EObjectTag GetTag() { return m_Tag; }
	inline bool HasTag(EObjectTag tag) const { return m_Tag == tag; }

	inline void Activate(bool bIsActive) { m_bIsActive = bIsActive; }
	inline bool GetActivie() { return m_bIsActive; }
	inline ObjectID GetID() { return m_nID; };
	inline EObjectModelType GetType() { return m_objType; }

	//오브젝트 풀링에서 풀에 되돌려 놓을때 호출되는 함수 
	void ResetState();
	
protected:

	//예제에서는 이걸 특정한 오브젝트 처리를 위해서만 하는거 같다
	//정말 반복되는 오브젝트한테는 이걸 안넣어준듯 한데
	ComPtr<ID3D12Resource> m_pd3dcbObject_Info;
	CB_GAMEOBJECT_INFO* m_pcbMappedObject_Info;

	
	CMesh* m_pMesh = nullptr; // 오브젝트는 자신의 형태를 가져야 함으로 Mesh를 갖는다.
	CShader* m_pShader = nullptr; // 자신을 어떻게 그릴지 어떤 Shader로 연결할지 PSO를 연결할지 등등을 결정짓는 Shader 클래스 이건 오브젝트가 가져도 되고 Shader 클래스가 오브젝트를 가져도 된다.
	CMaterial* m_pMaterial = nullptr;// Texture와 Shader를 갖고있을 예정 위에 Shader는 쓰이지 않을 것이다.

	int m_nMaterials = 0;
	CMaterial** m_ppMaterials = NULL; //GameObject가 계층구조로 존재하기에 Material
	//Part 별로 따로 존재할 수 있다.
public:
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh); // Mesh를 설정
	virtual void SetShader(CShader* pShader); // Shader를 설정
	virtual void SetMaterial(CMaterial* pMaterial);
	virtual void SetMaterial(int nMaterial ,CMaterial* pMaterial);

	virtual void Animate(float fTimeElapsed);  // Animate로 동작, 충돌 처리 등등 물리적인 연산등을 Render 이전에 수행시킬 것이며 GameObject들은 모두 상속 받아 독자적으로 동작할꺼라 virtual 을 써준다.
	virtual void OnPrepareRender(); //Direct12에서는 CommanndList 명령어를 수행하기 이전에 명시적으로 연결해줘야하거나 상태를 지정해줘야 함으로 PrepareRender와 같은 동작을 해주는 것이 필요하다.
	//현재는 PSO나 루트 시그니쳐 연결등을 여기서 처리하는걸 감안하지 않고 의미만 두고 연결해둔 부분 같다. 2025.07.04

	//
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }
	UINT64 GetCbvGPUDescriptorHandlePtr() { return(m_d3dCbvGPUDescriptorHandle.ptr); }
	

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera); //Animate -> Render로 실제 Draw콜을 실행하는 명령어 현재 이상태로는 Scene에 배치된 모든 GameObject 갯수만큼 DrawCall이 발생할 것이다.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera , bool bIsHir , bool bIsNormal, bool bIsWire, bool bIsShadow);
	virtual void DebugRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

public:
	//상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	//위에는 디스크립터 테이블에 상수버퍼를 담은 경우 아래는 그냥
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void ReleaseShaderVariables();

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetToParentPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	//게임 객체의 크기를 조정한다.
	void SetScale(float x, float y, float z);
	
	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//게임 객체를 월드 좌표 기준 방향으로 움직이게 한다
	void Move(XMFLOAT3 xmf3Offset);
	//게임 객체를 회전(x-축, y-축, z-축)한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void SetRotation(float fPitch, float fYaw, float fRoll);

public:

	char							m_pstrFrameName[64]; //해당 오브젝트의 프레임 이름

	CGameObject* m_pParent = NULL; //부모
	CGameObject* m_pChild = NULL; //자식
	CGameObject* m_pSibling = NULL; // 형제 오브젝트

	CAnimationController*			m_pSkinnedAnimationController = NULL;

	//Texutre의 갯수를 읽어오는 함수
	static int LoadCountTexturesFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

	static int LoadFrameHierarchyDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);


	//애니메이션 정보와 mesh 정보 등을 읽어오도록 호출 하는 함수.
	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader , EObjectModelType objectType);

	//계층 구조로 이루어진 오브젝트를 init 하는 함수
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes);


	//기본적으로 모든 검색은 부모 -> 형제 -> 자식 깊이 탐색 트리에 형태를 뛸것이다.
	CGameObject* GetParent() { return(m_pParent); } //해당 오브젝트의 부모를 반환


	virtual void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL); //계층 구조에 맞추어 트리를 따라 움직임을 업데이트 한다
	
	CGameObject* FindFrame(char* pstrFrameName); // 특정 위치에 오브젝트를 검색하거나 한다
	//이 FindFrame 기능을 이제 좀 더 빠르게 한다거나 개선할 수 있는 방식이 다양하게 추가해 줘야한다.
	//이유 1. Find 기능이 게임 구성에 있어서 굉장히 많이 쓰이는 사항
	// ++ 앞으로 Tag 기능을 추가하여 Tag 검색 조건을 추가하여 좀 더 검색 조건을
	// 좁혀 인게임내 검색을 좀 더 원활하고 빠르게 할 계획.
	// 또 다른 사항으론 형제 내 검색, 바로 아래 자식에서만 검색 등등 최적화하는
	//방법들을 상용엔진에서도 지원중에 있다.

	//
	CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	//Mesh 종류에 따라서 Shader나 InputLayout이나 설정이 달라져야 함으로 필요함.
	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); } 

	//재질 정보를 파일로 부터 읽어온다. 보통 파일에 어느 Texture를 어디에 바인딩할지
	//등등 정보가 담겨있다
	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//더미
	int LoadMaterialsDummyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

	//애니메이션이 포함된 오브젝트는 애니메이션 정보도 같이 읽어온다
	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);

	//현재 오브젝트에 Frame 정보를 출력
	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

	//자식을 설정
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	public:
		void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

		//
		void SetTrackAnimationClip(int nAnimationTrack, int nAnimationClip);
		//
		void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

		void SetRootMotion(bool bRootMotion);

		void PrintMatrixWorld() const;
		void PrintOBB() const;
		void PrintMatrixToParent() const;
		
		virtual void OnPrepareAnimate() { }
		virtual void OnLateUpdate() { }
	
public:
		void FillProtoTransform(Protocol::Transform* pTrans);

		XMFLOAT4 GetQuaternion() const;
		void SetQuaternion(const XMFLOAT4& quat);
};


class CRotatingObject : public CGameObject //GameObject 중 회전을 하는 오브젝트
{

public:
	CRotatingObject();
	virtual ~CRotatingObject();

	virtual void Init() {};
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;
public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; } //Speed값을 설정
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { // 회전 각을 설정
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}

	virtual void Animate(float fTimeElapsed);

public:

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle); //

};


///////////////////////////////////////////// 이 아래로 구조 개선
/*
* TOWILL:
* 위에 CGameObject에 계층 구조, 초기화 등등은 아래가 가져 가겠지만
* 이제 다른 Class와에 관계에 따라 쪼개서 늘릴 예정
* 나중에 어느정도 정리가 끝나면  인터페이스 등을 이용한 리팩토링도 가미
* 
* Material이 Shader를 갖는 구조로 구성을 할 것이며 ==> Shader는
* GraphicsCommon에서 초기화한 PSO들을 특정 Shader 구조에 맞춰 갖고만
* 있는 관계로 변경
* 
* 1. GameObject가 Material를 갖고있는 구조
* - CDefaultGameObject 클래스를 상속 받는 구조 
* 
* 2. Material이 GameObject를 갖고 있는 구조
* - CFromMaterialObject에 해당하는 구조
* 
* xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
* 여기 내용에서 CDefaultGameObject은 사용 X load 하는 부분에서
* 부모 관계가 틀어짐.
*/

class CDefaultGameObject : public CGameObject
{
public:
	CDefaultGameObject() : CGameObject(){};
	CDefaultGameObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);
	virtual ~CDefaultGameObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel);



};

class CFromMaterialObject : public CGameObject //
{
	//여긴 좀 나중에

};

///////////////////////////////////////////////////////////////////////

class CDefulatHumanAnimationCallbackHandler : public CAnimationCallbackHandler
{
private:
	CAnimationController* m_pController = NULL;

public:
	CDefulatHumanAnimationCallbackHandler(CAnimationController* pController) { m_pController = pController; }
	~CDefulatHumanAnimationCallbackHandler() { delete m_pController; }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

//Test용
class CAnimatedObject : public CGameObject
{
	//TODO : 
public:
	/*
	* 1. ID3D12Device
	* 2. ID3D12GraphicsCommandList
	* 3. ID3D12RootSignature
	* 4. LoadModel
	* 5. nAnimationTracks
	*/
	CAnimatedObject() {};
	CAnimatedObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CAnimatedObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);

	void SetAnimationState(EAnimationState State); //애니메이션에 현 상태를 받아와 반영하는 함수

};

///////////////////////////////////////////////////////////////////////

class CWoodManObject : public CAnimatedObject
{
	
public:

	CWoodManObject() {};
	CWoodManObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CWoodManObject();

	virtual void Init(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};

///////////////////////////////////////////////////////////////////////

class CSkyBoxObject : public CGameObject
{
public:
	CSkyBoxObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wchar_t* pwstrSkyBoxHDRPath);
	virtual ~CSkyBoxObject();
};

///////////////////////////////////////////////////////////////////////

class CPlanObject : public CGameObject
{
public:
	CPlanObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CPlanObject();
};
//////////////////////////////////////////////////
//

class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage* m_pHeightMapImage;

	int								m_nWidth;
	int								m_nLength;

	XMFLOAT3						m_xmf3Scale;

	int m_nSeasonIndex = 0;// 0,1,2,3 계절을 Terrain에 분기해서 알려주기 위해

public:
	float GetHeight(float x, float z) { return(m_pHeightMapImage->GetHeight(x, z) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow);
	
	void SetSeason(int num) { m_nSeasonIndex = num; }
};


//////////////////////////////////////////////////
//

class CCropObject : public CDefaultGameObject
{
public:
	CCropObject() {};
	CCropObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel) :CDefaultGameObject() {};

	virtual ~CCropObject() {};

	virtual void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel) {};

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};

	void CCropObject::ChangeModel(CLoadedModelInfo* pNewModel);
	void SetGrowth(float num) { m_fGrowth = num; }
	float GetGrowth() { return m_fGrowth; }

	void AddHP(int num) { m_nHP += num; }
	void SetHP(int num) { m_nHP = num; }
	int GetHP() { return m_nHP; }

protected:
	float m_fGrowth = 0.0f;
	int m_nHP = 1; // 체력


};

//////////////////////////////////////////////////
//
// 옥수수
class CCornObject : public CCropObject
{
public:
	CCornObject() {};
	CCornObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CCornObject();

	virtual void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// 감자
class CPotatoObject : public CCropObject
{
public:
	CPotatoObject() {};
	CPotatoObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CPotatoObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// 호박
class CPumpkinObject : public CCropObject
{
public:
	CPumpkinObject() {};
	CPumpkinObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CPumpkinObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);


};


//////////////////////////////////////////////////
// 토마토
class CTomatoObject : public CCropObject
{
public:
	CTomatoObject() {};
	CTomatoObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CTomatoObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);



};


//////////////////////////////////////////////////
// 간이 밭(Log Planter)
enum class ECropState
{
	Empty = 0,       // 아무것도없음
	Growing = 1,     // 자라는 중
	Harvestable = 2, // 수확 가능
	Dead = 3,         // 고사
};

class CLogPlnaterObject : public CDefaultGameObject
{
public:
	CLogPlnaterObject() {};
	CLogPlnaterObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CLogPlnaterObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	//현재 심어져있는 작물의 상태
	ECropState m_cropState = ECropState::Empty;

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};


//////////////////////////////////////////////////
// 헛간
class CBarnObject : public CDefaultGameObject
{
public:
	CBarnObject() {};
	CBarnObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CBarnObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);
	void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};


/////////////////////////////////////////////////////
// 

class CRockObject : public CDefaultGameObject
{
public:
	CRockObject() {};
	CRockObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CRockObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }
private:
	int m_nHP = 10; // 체력
};

/////////////////////////////////////////////////////
// 

class CWallObject : public CDefaultGameObject
{
public:
	CWallObject() {};
	CWallObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CWallObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//2면에 설치 했는지 안했는지
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;
};
/////////////////////////////////////////////////////
// 

class CCeilingObject : public CDefaultGameObject
{
public:
	CCeilingObject() {};
	CCeilingObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CCeilingObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	
	//4면에 설치 했는지 안했는지
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;
};
/////////////////////////////////////////////////////
// 

class CFloorObject : public CDefaultGameObject
{
public:
	CFloorObject() {};
	CFloorObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CFloorObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	
	//4면에 설치 했는지 안했는지
	UINT* m_npPlaceable = NULL;
	int m_nSelectIndex = -1;

};

/////////////////////////////////////////////////////
//나무 

class CTreeObject : public CDefaultGameObject
{
public:
	CTreeObject() {};
	CTreeObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CTreeObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }
private :
	int m_nHP = 5; // 체력
};

/////////////////////////////////////////////////////
// 풀

class CGrassObject : public CDefaultGameObject
{
public:
	CGrassObject() {};
	CGrassObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual ~CGrassObject();

	void Init(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature,
		CLoadedModelInfo* pModel);

	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	void AddHP(int num) { m_nHP += num; }
	int GetHP() { return m_nHP; }
	void SetHP(int num) { m_nHP = num; }

private:
	int m_nHP = 1; // 체력

};
//////////////////////////////////////////////////
/////////////////////////////////////////////////////
// 
class CParticleObject : public CGameObject
{
public:
	CParticleObject();
	virtual ~CParticleObject();

	virtual void UpdateShaderVariables(
		ID3D12GraphicsCommandList* pd3dCommandList,
		XMFLOAT4X4* pxmf4x4World);

	virtual void SetParticleState(
		float fTime,
		float fTimeElapsed,
		float fSpawnRadius,
		float fAlphaScale,
		UINT  nParticleFlags,
		XMFLOAT3 gParticleColor,
		UINT nBlockCount,
		WeatherBlock* blocks);

	void SetPreviousPosition(XMFLOAT3 pos);
	XMFLOAT3 GetPreviousPosition();
protected:
	ComPtr<ID3D12Resource> m_cpcbParticle;
	cbParticle* m_pcbParticle = nullptr;

	XMFLOAT3 m_xmf3PreviousPosition;
};

//파티클 오브젝트 (1) Rain

class CRainParticleObject : public CParticleObject
{
public:

	CRainParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CRainParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);


};
/////////////////////////////////////////////////////
// 파티클 오브젝트 (2) Snow

class CSnowParticleObject : public CParticleObject
{
public:
	CSnowParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CSnowParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);


};

/////////////////////////////////////////////////////
// 파티클 오브젝트 (3) Dust ==> 황사로 쓸 예정

class CDustParticleObject : public CParticleObject
{
public:
	CDustParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);
	virtual ~CDustParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

};

/////////////////////////////////////////////////////
// 파티클 오브젝트 (4) punch

class CPunchParticleObject : public CParticleObject
{
public:
	CPunchParticleObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, int nCount);

	virtual ~CPunchParticleObject();
	//virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	void Animate(float fTimeElapsed, float fTime);
	float m_fCircleTime = 0.0f;

	void PrintStartTIme();

	virtual void SetParticleState(
		float fTime,
		float fTimeElapsed,
		float fSpawnRadius,
		float fAlphaScale,
		UINT  nParticleFlags,
		XMFLOAT3 gParticleColor,
		UINT nBlockCount,
		WeatherBlock* blocks);

};


/////////////////////////////////////////////////////
//
struct PreOBBData 
{
	EObjectModelType m_objectType = EObjectModelType::None;
	CMesh* m_pOBBMesh = NULL;
	BoundingOrientedBox m_OBBBox;
	XMFLOAT3 m_xmf3Offset = XMFLOAT3(0, 0, 0);
};
class CBuildingObject : public CGameObject
{
public:
	CBuildingObject() {};
	CBuildingObject(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual ~CBuildingObject();

	PreOBBData* m_pOBBData = NULL;
	UINT m_nOBBData = 0;
	
	EObjectModelType m_objType = EObjectModelType::None;
	EObjectTag m_objTypeTag = EObjectTag::None;

	//현재 충돌된 오브젝트가 누군지 가르키는 변수.
	CGameObject* m_pHitGameObject = NULL;

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool bIsHir, bool bIsNormal, bool bIsWire, bool bIsShadow);
	//설치할 수 있는 모든 모델에 대하여 임시 OBB를 사전에 Data 저장해둔다
	virtual void SetOBB(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//SetOBB에서 설정한 Data를 기준으로 OBB를 세팅한다
	void ResetOBB(PreOBBData m_OBBData);

};

///////////////////////////////////////////////////////////////////////

class CPostProcessingObject : public CGameObject
{
public:
	CPostProcessingObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CPostProcessingObject();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

//////////////////////////////////////////////////
//

/////////////////////////////////////////////////////
// 오브젝트 풀링
template <typename T>
class CObjectPool 
{
private:
	vector<T*> m_vpList;

public:
	virtual ~CObjectPool()
	{
		for (T* obj : m_vpList)
		{
			delete obj;
		}
	}

	// ⭐ 가변 인자 템플릿
	template <typename... ARGS>
	T* Acquire(ARGS&&... args)
	{
		T* obj = nullptr;

		if (!m_vpList.empty())
		{
			//std::cout << "재사용 중" << endl;
			// 재사용 객체
			obj = m_vpList.back();
			m_vpList.pop_back();

			// 생성자는 이미 실행된 상태이므로 Init만 호출한다
			obj->Init(std::forward<ARGS>(args)...);
		}
		else
		{
			//std::cout << "새로 생성 중" << endl;
			// 최초 생성: new T( args... )
			obj = new T(std::forward<ARGS>(args)...);
		}
		//std::cout << m_vpList.size()<<"현재 풀링의 크기" << endl;
		obj->Activate(true);
	
		return obj;
	}

	void Release(T* obj)
	{
		obj->ResetState();
		obj->Activate(false);
		m_vpList.push_back(obj);
	}
};

