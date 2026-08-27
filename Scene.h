#pragma once
#include "GameTimer.h"
#include "Shader.h"
#include "SceneManager.h"
#include "GraphicsCommon.h";
#include "ClientNetwork.h";

struct PendingMoveInput
{
	uint32 sequence = 0;
	XMFLOAT3 predictedDelta = { 0.0f, 0.0f, 0.0f };
};

//나무, 바위, 풀을 배치하는 파싱용 데이터 구조
struct StageSpawnEntry
{	
	int temp_id;
	EObjectModelType type;
	CLoadedModelInfo* model;
	std::vector<XMFLOAT2> positions;
};

struct StageSpawnData
{
	int stageIndex;
	std::vector<StageSpawnEntry> entries;
};


//Light 구조체를 갖고있으며 충돌체와 초기화 관련해서 간편화를 위한 객체
class LightControl
{
public:
	LightControl();
	virtual ~LightControl();
	void SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, UINT nbIsUse );
	void SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, float fallOffStart, float fallOffEnd, float spotPower , UINT nbIsUse, UINT nbIsShadowUse);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetDirection(XMFLOAT3 xmf3Direction);
	void SetShadowUse(UINT nbIsUse);
	
	Light GetLight();
	BoundingSphere GetBoundingSphere();

private:
	Light m_Lights;
	BoundingSphere spotSphere;
};


//UpdateShaderVariables에서 사용중
struct Toggle
{
	bool* state;
	UINT flag;
};

struct ImGuiMaterialFlages //ImGui를 통해 Shader에 전달할 CBV에 내용을 담을 것.
{
	UINT							m_nMapType = 0x00; // 어떤 Map을 사용할지
	XMFLOAT3					m_pDummy = XMFLOAT3(0.0f, 0.0f, 0.0f);
	UINT							m_nLightType = 0x00; // 조명 계산 방식을 어떤걸 적용할지.
	XMFLOAT3					m_pDummy2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float							m_fpExposure = 0.0f;
	float							m_fpGamma = 0.0f;
	float							m_fpRoughness = 0.0f;
	float							m_fpMetalness = 0.0f;
	UINT							m_nShadowOnOFF = 0; //False : 0, Ture : 1
	UINT							m_nSeasonType = 0x00; //어떤 계절인지
	UINT							m_nWeatherType = 0x00; //어떤 날씨인지
};

struct ImGuiComputeFlages
{
	XMFLOAT4X4 m_xmf4x4invPorj;
	XMFLOAT4X4 m_xmf4x4Proj;
	float m_xmf2Radius;
	float m_fBias;//
	UINT m_nType; //SSAO, Bloom등을  On/Off 해줄지
	float BloomThreshold;
	float BloomSoftKnee;
	float BloomIntensity;
};

class CDescriptorHeap
{
public:
	CDescriptorHeap();
	~CDescriptorHeap();

	ComPtr<ID3D12DescriptorHeap> m_pd3dCbvSrvDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dUavGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dCbvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dCbvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dUavCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dUavGPUDescriptorNextHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUUavDescriptorStartHandle() { return(m_d3dUavCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUUavDescriptorStartHandle() { return(m_d3dUavGPUDescriptorStartHandle); }
};


class CScene
{
public:
	CScene(HWND hWnd, CGameTimer* timer, unique_ptr<ClientNetwork> clienteNetwork);
	virtual ~CScene();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);	//루트 시그니쳐를 생성해주는 함수 ==> 이 둘은 나중에

	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);	//PSO를 만들어주는 함수. ==> 외부 클래스 혹은
	//stdafx로 빼내거나 아니면 Scene Manager를 만들어 별개로 빼둘 계획이다. 여러 Scene을 Load하고 관리할려면
	//생성관련된 함수는 공용으로 설계하는게 가장 좋으니까.
	//아니면 이 Scene 클래스를 Scene Manager 처럼 사용하고 다른 Scene 클래스를 만들어서 설계해도 좋을 듯 하다.
	//GameFramework에서 모두 한 번에 처리하도록 설계하는 것도 나쁘지 않을듯??
	//물론 Scene에 사용되는 리소스에 량에 따라서 혹은 다른 루트 PSO 가 필요한 경우가 있으니 이건 좀 더 고려해 봐야 한다.
	//이 문구는 항상 읽어봐라 2025.02.15 작성
	// Scene구조를 스택 구조로 바꾸어 사용할 예정으로 변경.
	// 루트 시그니쳐와 루트 파라미터와 같은것들은 별계의 유틸 클래스로 구별하여 static으로 만들어 상황에따라 Scene에서 선택하도록 변경예정
	// 이 문구는 2025.05.31 작성


	////////////Texture적용을 위해서

	/*
	* CreateCbvSrvDescriptorHeaps를 먼저 선언해서, 상수버퍼 뒤에 SRV를 바인딩할
	* 포인터 위치를 지정해준다
	*
	*/

	CTexture* m_pRenderTexture; // RTV, SRV, UAR 등등 활용할때 사용할 친구
	CTexture* m_pComputeRenderTexture; // SSAO에 G-buffer로 사용해서 넘겨줄 Render Texture

	CTexture* m_pShadowDepthTexture;


	static CDescriptorHeap* m_pDescriptorHeap;

	static void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews);
	static void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT* pdxgiFormat,UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	static void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateStructureBufferSRV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource,UINT nElementCount,UINT nStructureStride);

	static D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat); //깊이맵 같이 따로 리소스를 갖고있는 친구에 경우 이걸로 처리
	//UAV
	static void CreateUnorderedAccessView(ID3D12Device* pd3dDevice,CTexture* pTexture, DXGI_FORMAT* pdxgiFormat,UINT nDescriptorHeapIndex,UINT nRootParameterStartIndex);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateStructureBufferUAV(ID3D12Device* pd3dDevice,ID3D12Resource* pd3dResource,UINT nElementCount,UINT nStructureStride);

	//Render Target view와 Srv를 동시에 연결해주는 함수
	void CreateShaderResourceViewRtvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle);
	
	void CreateShaderResourceViewAndUAVStructuredBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);

	//Shadow Map Create Fun
	void CreateShaderResourceViewDsvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nDepthTextures, D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle , ID3D12Resource* DefaultDepthStncilBuffer = NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE* m_pd3dRtvCPUDescriptorHandles = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE* m_pd3dG_bufferRtvCPUDescriptorHandles = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE* m_pd3dSahdowMapDsvCPUDescriptorHandles = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE m_pd3dDefaultDepthMapSrvDescrptorHandles; //기본 깊이맵의 SRV Handle

	ID3D12Resource* m_pDefaultDepthStancilResource = NULL;

	ID3D12Resource* GetRenderTextureResource(UINT nIndex) { return(m_pRenderTexture->GetResource(nIndex).Get()); }
	ID3D12Resource* GetG_BufferRenderTextureResource(UINT nIndex) { return(m_pComputeRenderTexture->GetResource(nIndex).Get()); }
	ID3D12Resource* GetShadowMapResource(UINT nIndex) { return(m_pShadowDepthTexture->GetResource(nIndex).Get()); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCPUDescriptorHandle(UINT nIndex) { return(m_pd3dRtvCPUDescriptorHandles[nIndex]); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetG_BufferRtvCPUDescriptorHandle(UINT nIndex) { return(m_pd3dG_bufferRtvCPUDescriptorHandles[nIndex]); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvCPUDescriptorHandle(UINT nIndex) { return(m_pd3dSahdowMapDsvCPUDescriptorHandles[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetDsSrvGPUDescriptorHandle() { return(m_pd3dDefaultDepthMapSrvDescrptorHandles); }

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle); }
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle); }
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle); }

	//////////////
	virtual void BuildObjects(ID3D12Device* pd3dDevice);
	virtual void BuildObjects(ID3D12Device* pd3dDeivce, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void InitLights();

	virtual void ReleaseObjects(); //항상 생성하는 오브젝트가 존재한다면, 삭제도 당연히 존재해야한다.

	virtual void PreviousTransform(); //이전 프레임에 동작등을 저장처리할 함수
	virtual bool ProcessInput();	//
	virtual void AnimateObjects(float fTimeElapsed);	//

	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);	//Render를 하기전 PSO나 루트 시그니쳐 등을
	//연결을 해줘야 하는데 이러한 공통 설정등을 미리 연결해주는걸 항상 해야 한다.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);		//

	//Shadow를 그릴 친구들만
	virtual void OnlyShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void PrePareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList);
	//그림자를 그리기 위한 준비 혹은 그리는 과정 등등 메인 오브젝트 그리기전 동작
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList,int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, 
		D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera);
	//메인 동작
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);	//Render를 할때 여러 카메라가 존재할 수 있고, 카메라에 따라 값이 달라질 수 있음으로 Camera를 파라미터로 받는다.
	//최종적으로 출력 및 포스트 프로세싱 동작을 위한 Render 
	virtual void OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles
		, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle,CCamera* pCamera);

	virtual void OnGUI(ImGuiIO* pGuiIO); //Scene마다 GUI 설정을 달리 하기 위해서

	virtual void ReleaseUploadBuffers();

	//virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice); //루트 시그니쳐를 생성해주고 반환
	virtual ID3D12RootSignature* GetGraphicsRootSignature(); //저장하고 있는 루트 시그니쳐를 반환
	
	/*
	* ImGui 설정을 넘기기 위한 ShaderVariables로 register번호는 항상 4를 유지한다
	*/
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	ComPtr<ID3D12RootSignature> m_pd3dRootSignature;	//루트 시그니쳐
	ComPtr<ID3D12RootSignature> m_cpd3dComputeRootSignature;//컴퓨트 파이프라인을 위한 루트 시그니쳐
	ComPtr<ID3D12PipelineState> m_pd3dPipelineState;	//그래픽 파이프라인 만일 Compute 파이프라인이면 같은 변수지만 별계로
	//내부를 설정해줘야 한다.

	ComPtr<ID3D12Device> m_cpd3dDevice;
	ComPtr<ID3D12GraphicsCommandList> m_cpd3dCommandList;


	//Scene의 Stack 변화에 따른 상태 변화를 알리는 것들.
	virtual void OnEnter() { std::cout << "Scene OnEnter"<<endl; };
	virtual void OnPause() { std::cout << "Scene OnPause" << endl; };
	virtual void OnResume() { std::cout << "Scene OnResume" << endl; };
	virtual void OnExit() { std::cout << "Scene OnExit" << endl; };

	CGameTimer* m_pGameTimer;
	HWND m_hWnd;

	float								m_fElapsedTime = 0.0f;
	float								m_fTime = 0.0f; //현재 지나가고 있는 시간
	//
public:

	//계층 구조를 위해 추가한 GameObject
	int m_nHierarchicalGameObjects = 0;
	vector<CGameObject*> m_ppHierarchicalGameObjects ;


	ObjectID m_nIDPool = 0; //클라이언트 자체적으로 ID가 겹치지 않도록 계속 추가 되는 애들은 ++될 예정

	CObjectPool<CAnimatedObject> m_AnimatedObjectPool; //기본 애니메이션 오브젝트 풀
	CObjectPool<CDefaultGameObject> m_DefaultObjectPool; //기본 일반 오브젝트 풀

	//오브젝트 풀링을 사용해서 오브젝트를 초기화 및 생성 그리고 제거 하는 함수들
	template<typename T>
	T* SpawnAnimateObject(CObjectPool<T>& objectPool, CLoadedModelInfo* pPrefab, EObjectTag tag, ObjectID id, int nAnimationTracks)
	{
		T* obj = objectPool.Acquire(m_cpd3dDevice.Get(), m_cpd3dCommandList.Get(), m_pd3dRootSignature.Get(),
			pPrefab, nAnimationTracks);

		obj->SetTag(tag); //논리적 Tag

		obj->SetID(id); //고유 ID

		obj->SetType(pPrefab->m_ObjectType); //어떤 모델 형태의 Type을 사용하고있는지 지정.

		if (m_nHierarchicalGameObjects >= m_ppHierarchicalGameObjects.size())
		{
			// 넘쳤으니까 자동 확장
			m_ppHierarchicalGameObjects.push_back(obj);
		}
		else
		{
			m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects] = obj;
		}

		m_nHierarchicalGameObjects++;

		return obj;
	}
	
	template<typename T>
	T* SpawnDefaultObject(CObjectPool<T>& objectPool, CLoadedModelInfo* pPrefab, EObjectTag tag, ObjectID id)
	{
		T* obj = objectPool.Acquire(m_cpd3dDevice.Get(), m_cpd3dCommandList.Get(), m_pd3dRootSignature.Get(),
			pPrefab);

		obj->SetTag(tag);

		obj->SetID(id);

		obj->SetType(pPrefab->m_ObjectType);

		if (m_nHierarchicalGameObjects >= m_ppHierarchicalGameObjects.size())
		{
			// 넘쳤으니까 자동 확장
			m_ppHierarchicalGameObjects.push_back(obj);
		}
		else
		{
			m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects] = obj;
		}

		m_nHierarchicalGameObjects++;

		return obj;
	}
	 
	 template<typename T>
	 void DeSpawnAnimateObject(CObjectPool<T>& objectPool, CGameObject* obj)
	 {
		 for (int i = 0; i < m_nHierarchicalGameObjects; i++)
		 {
			 if (m_ppHierarchicalGameObjects[i] == obj)
			 {
				 m_ppHierarchicalGameObjects[i] = m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects - 1];
				 m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects - 1] = nullptr;
				 m_nHierarchicalGameObjects = m_nHierarchicalGameObjects - 1;
				 break;
			 }
		 }

		 T* pAnimated = dynamic_cast<T*>(obj);
		 objectPool.Release(pAnimated);
	 }

	template<typename T>
	void DeSpawnDefaultObject(CObjectPool<T>& objectPool, CGameObject* obj)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++)
		{
			if (m_ppHierarchicalGameObjects[i] == obj)
			{
				m_ppHierarchicalGameObjects[i] = m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects - 1];
				m_ppHierarchicalGameObjects[m_nHierarchicalGameObjects - 1] = nullptr;
				m_nHierarchicalGameObjects = m_nHierarchicalGameObjects - 1;
				break;
			}
		}
		T* pDefault = dynamic_cast<T*>(obj);
		objectPool.Release(pDefault);
	}

	//const float m_fDelay = 1;
	//float m_ftimes = 0;
	virtual void SetPlayer() {};
	virtual void SetNetWorkPlayer() {};



protected:

	//CTexture* m_pRenderTexture = NULL; // RTV, SRV, UAR 등등 활용할때 사용할 친구
	unique_ptr<ClientNetwork> m_upClientNetwork;

	ComPtr<ID3D12Resource> m_cpd3dcbImGui_Info;
	ImGuiMaterialFlages* m_pcbMaterialFlages = NULL;

	ComPtr<ID3D12Resource> m_cpd3dcbImGui_ComputeInfo;
	ImGuiComputeFlages* m_pcbComputeFlages = NULL;

	ComPtr<ID3D12Resource> m_cpd3dcbLights = NULL;
	Light* m_pcbMappedLights = NULL;

	ComPtr<ID3D12Resource> m_cpd3dcbSSAOKernel;
	SSAOKernelCB* m_pcbSSAOKernel;


	//CShader안에도 오브젝트가 있는데 밑에 오브젝트를 사용하는 동안에는 쓰지 않아야 한다.
	CShader** m_ppShader = NULL;
	//int m_nShaders = 0;

	//배치(Batch) 처리를 하기 위한 Shader
	CObjectsShader* m_pShaders = NULL;
	int m_nShaders = 0;

	//게임 오브젝트를 씬이 직접 가지고 있는 구조로 바꿔서 테스트하기 위해 추가된 부분.
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;

	CGameObject* m_pTestObject = NULL;

	//

	int m_nCBVCount = 0; //CBV Heap에 들어갈 CBV 갯수 오브젝트 갯수
	int m_nSRVCount = 0; //Texture 몇개 쓸껀지

	//현재 Material에 어떤 TextureMap을 적용시킬지 말지 On/OFF


	public:
		//파일로 부터 Texture의 총 갯수를 읽어와 반환해주는 것.
		int CountTexturesFromFiles(ID3D12Device* pd3dDevice,
			ID3D12GraphicsCommandList* pd3dCommandList,
			ID3D12RootSignature* pd3dGraphicsRootSignature, const std::vector<const char*>& fileList);

	public:
		//ImGui에서 Flag 값으로 재질, 조명 영향을 어느것들을 적용할지 연결해둔 
		void SetMaterialType(UINT ntype) { m_pcbMaterialFlages->m_nMapType |= ntype; }
		void UnsetMaterialType(UINT nType) { m_pcbMaterialFlages->m_nMapType &= ~nType; }
		void SetLightType(UINT ntype) { m_pcbMaterialFlages->m_nLightType |= ntype; }
		void UnsetLightType(UINT nType) { m_pcbMaterialFlages->m_nLightType &= ~nType; }
		void SetSeasonType(UINT ntype) { m_pcbMaterialFlages->m_nSeasonType |= ntype; }
		void UnsetSeasonType(UINT nType) { m_pcbMaterialFlages->m_nSeasonType &= ~nType; }
		void SetWheatherType(UINT ntype) { m_pcbMaterialFlages->m_nWeatherType |= ntype; }
		void UnsetSetWheatherType(UINT ntype) { m_pcbMaterialFlages->m_nWeatherType &= ~ntype; }
		void SetComputeFlagType(UINT ntype) { m_pcbComputeFlages->m_nType |= ntype; }
		void UnsetComputeFlagType(UINT ntype) { m_pcbComputeFlages->m_nType &= ~ntype; }

		bool m_bIsUseAlbedo = true;
		bool m_bIsUseNormal = true;
		bool m_bIsUseRoughness = false;
		bool m_bIsUseMetalness = false;
		bool m_bIsUseAO = false;
		bool m_bIsUseSpecularF0 = false;
		bool m_bIsUseEmission = false;

		bool m_bIsUsedirect = true;
		bool m_bIsUseambient = false;
		bool m_bIsUseDirectionLight = true;

		//계절
		int m_nSeasonIndex = 0; // spring : 0, Summer : 1, Fall : 2, Winter : 3

		int m_nBuildingObjectIndex = 0; // Wall : 0, Floor : 1, Ceiling : 2 
		int m_nPreviousBuildingObjectIndex = -1; //이전에 선택했던거

		//날씨
		int m_nWeather = 0;  // Sunny : 0, rain : 1, snow : 2, dust : 3, 4 : Fog

		//ToonMapping을 위한 부분
		float m_fpExposure = 1.0f;
		float m_fpGamma = 2.2f;

		//PBR 테스트
		float m_fpRoughnessValue = 0.0f;
		float m_fpMetalness = 0.0f;

		//Shadow On/Off
		bool m_bIsShadow = true;

		//SSAO, BLOOM
		bool m_bIsUseSSAO = false;
		bool m_bIsUseBLOOM = false;
		float m_fRadiusX = 0.5f;
		float m_fRadiusY = 0.5f;
		float m_fBias = 0.025;
		float m_fBloomThreshold = 1.0f;
		float m_fBloomSoftKnee = 0;
		float m_fBloomIntensity = 1.0f;

		//점수
		float m_fScore;


		int m_nLights = 0;
		Light* m_pLights = NULL;
		vector<LightControl> m_vectorLightControl;
		//vector<Light> m_vectorLights; //동적 조명으로 조명 모델 오브젝트를 추가하고나서 활약할 예정

		//IBL을 위한 부분
		CTexture* m_pIBLBRDFTexture = NULL;
		CTexture* m_pIBLSpecluarTexture = NULL;
		CTexture* m_pIBLIrradianceTexture = NULL;

		CTexture* m_pIBLBRDFTextureCloud = NULL;
		CTexture* m_pIBLSpecluarTextureCloud = NULL;
		CTexture* m_pIBLIrradianceTextureCloud = NULL;


		//obb는 부딪친 오브젝트의 obb, point는 플레이어의 obb 중심점 
		XMVECTOR ClosetPointOnOBB(BoundingOrientedBox& obb, XMVECTOR point);

		virtual void CreateSSAOKernel();

		//클러스터드 렌더링에 사용할 구조체 버퍼 GPU 핸들
		D3D12_GPU_DESCRIPTOR_HANDLE m_hClusterLightListUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hClusterLightListSrv;
		
		ComPtr<ID3D12Resource> m_cpcbLightData;
		LightingData* m_pcbLightData = nullptr;
		LightingData* m_pLightData = NULL;

		ComPtr<ID3D12Resource> m_cpsbClusterLightList;
		ClusterLightList* m_psbClusterLightList = nullptr;

		ComPtr<ID3D12Resource> m_cpcbCluster;
		ClusterCB* m_pcbCluster = nullptr;

		XMUINT3 m_ClusterDim = XMUINT3(32, 32, 24); //어떻게 클러스터드를 쪼갤지

		public:
			void SendObjectMoveToServer(CGameObject* pObj, Protocol::PlayerState state,
				float inputX = 0.0f, float inputZ = 0.0f, float deltaTime = 0.0f);
			void ReconcileLocalPlayer(CGameObject* pObj, const XMFLOAT3& serverPosition,
				uint32 lastProcessedSequence);

			// Login
			int m_szLoginID =  0;
			int m_szLoginPW =  0;

			bool m_bLoginFailed = false;
			bool m_bLoginRequesting = false;
			bool m_bShowLoginUI = true;

			uint32 m_nextMoveSequence = 1;
			XMFLOAT3 m_lastPredictedMovePosition = { 0.0f, 0.0f, 0.0f };
			bool m_hasPredictedMovePosition = false;
			vector<PendingMoveInput> m_pendingMoveInputs;
			std::mutex m_movementReconciliationMutex;
			CGameObject* m_pMovementCorrectionTarget = nullptr;
			XMFLOAT3 m_movementCorrection = { 0.0f, 0.0f, 0.0f };
			CGameObject* m_pLastMovePacketObject = nullptr;
			Protocol::PlayerState m_lastMovePacketState = Protocol::PlayerState::IDLE;
			uint64 m_lastMovePacketTimeMs = 0;
			bool m_wasAirborneForNetwork = false;

		
};
