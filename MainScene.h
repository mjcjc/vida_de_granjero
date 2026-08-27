#pragma once
#include "Scene.h"
#include "Player.h"
#include <mutex>
#include <deque>
#include <unordered_map>
///////////////////////////////////// <summary>//////////////////////////////////////
/// 어쩔 수 없이 보내려고 만들어 둔 거니 이해좀 해주십쇼...
struct WorldSpawnInfo 
{
	uint32 objId;
	uint32 type;
	XMFLOAT3 positions;
};

struct RemoteMovementSnapshot
{
	uint64 arrivalTimeMs = 0;
	uint32 serverTick = 0;
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
	Protocol::PlayerState state = Protocol::PlayerState::IDLE;
};
///////////////////////////////////// <summary>//////////////////////////////////////
class CMainScene : public CScene 
{
public:

	CMainScene(HWND hWnd, CGameTimer* timer, unique_ptr<ClientNetwork> clienteNetwork) : CScene(hWnd, timer, std::move(clienteNetwork))
	{
		m_hWnd = hWnd;
		m_pGameTimer = timer;
		m_vpCollisionIndex.resize(10, -1);
	};
	virtual ~CMainScene() {};

	CSkyBoxObject* m_pSkyBox = NULL;
	CSkyBoxObject* m_pSkyBoxCloud = NULL;

	//파티클
	CRainParticleObject* m_pRainParticle = NULL;
	CSnowParticleObject* m_pSnowParticle = NULL;
	CDustParticleObject* m_pDustParticle = NULL;
	CPunchParticleObject* m_pPunchParticleObject = NULL; //플레이어가 펀치를 날렸을때 무언가 부딪치면 튀기는 파티클

	CPlanObject* m_pPlan = NULL;

	CHeightMapTerrain* m_pTerrain = NULL;

	CGameTimer* m_pGameTimer;

	HWND m_hWnd;

	POINT m_ptOldCursorPos;

	//GameFramwwork에 있는 카메라와는 별개에 카메라로 Scene에 Render에 넘어온 pCamera는 현재 사용중이 아니다
	CCamera* m_pCamera = NULL;
	CPlayer* m_pPlayer = NULL;

	////계층 구조를 위해 추가한 GameObject
	//int m_nHierarchicalGameObjects = 0;
	//CGameObject** m_ppHierarchicalGameObjects = NULL;

	CPostProcessingObject* m_pPostProcessingObject = NULL;

	bool m_bIsShowNormals = false;
	bool m_bIsShowWire = false;
	bool m_bIsShowBoundingBox = false;

	UINT m_ShadowIndex = 0;

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);	//루트 시그니쳐를 생성해주는 함수 ==> 이 둘은 나중에

	//Texture를 추가한 Scene에 내용을 추가하기 위하여 Scene들은 개별로 가져야할 필요가 있음
	//흠 
	//virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	virtual void BuildObjects(ID3D12Device* pd3dDeivce, ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void InitLights();

	virtual void ReleaseObjects(); //항상 생성하는 오브젝트가 존재한다면, 삭제도 당연히 존재해야한다.

	virtual void PreviousTransform(); //이전 프레임에 동작등을 저장처리할 함수
	virtual bool ProcessInput();	//

	void PrintLightPositions(const Light* pLights, int lightCount);

	bool bIsFirstAnimate = false;
	
	virtual void AnimateObjects(float fTimeElapsed);	//
	
	virtual void OnlyShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void Lightsort(Light* pLights, LightingData* pLightingdata,CCamera* pCamera , vector<LightControl> vectorLightControl);
	virtual void PrePareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	//virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);	//Render를 할때 여러 카메라가 존재할 수 있고, 카메라에 따라 값이 달라질 수 있음으로 Camera를 파라미터로 받는다.
	virtual void OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles
		, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera);
	
	virtual void OnGUI(ImGuiIO* pGuiIO);

	virtual void ReleaseUploadBuffers();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) ;

	virtual void OnEnter() ;
	virtual void OnPause() { std::cout << "Scene OnPause" << endl; };
	virtual void OnResume() { std::cout << "Scene OnResume" << endl; };
	virtual void OnExit() { std::cout << "Scene OnExit" << endl; };

public:
	//Prefab과 같은것
	CLoadedModelInfo* m_pWoodManModel = NULL;
	CLoadedModelInfo* m_pWorkManModel = NULL;

	// Corn
	CLoadedModelInfo* m_pCorn01Model = NULL;
	CLoadedModelInfo* m_pCorn02Model = NULL;
	CLoadedModelInfo* m_pCorn03Model = NULL;
	CLoadedModelInfo* m_pCorn04Model = NULL;
	CLoadedModelInfo* m_pCornSeedModel = NULL;

	// Potato
	CLoadedModelInfo* m_pPotato01Model = NULL;
	CLoadedModelInfo* m_pPotato02Model = NULL;
	CLoadedModelInfo* m_pPotato03Model = NULL;
	CLoadedModelInfo* m_pPotato04Model = NULL;
	CLoadedModelInfo* m_pPotatoSeedModel = NULL;

	// Pumpkin
	CLoadedModelInfo* m_pPumpkin01Model = NULL;
	CLoadedModelInfo* m_pPumpkin02Model = NULL;
	CLoadedModelInfo* m_pPumpkin03Model = NULL;
	CLoadedModelInfo* m_pPumpkin04Model = NULL;
	CLoadedModelInfo* m_pPumpkinSeedModel = NULL;

	// Tomato
	CLoadedModelInfo* m_pTomato01Model = NULL;
	CLoadedModelInfo* m_pTomato02Model = NULL;
	CLoadedModelInfo* m_pTomato03Model = NULL;
	CLoadedModelInfo* m_pTomato04Model = NULL;
	CLoadedModelInfo* m_pTomatoSeedModel = NULL;

	// Others
	CLoadedModelInfo* m_pLogPlanterModel = NULL;
	CLoadedModelInfo* m_pBarnModel = NULL;
	CLoadedModelInfo* m_pRock01Model = NULL;
	CLoadedModelInfo* m_pRock02Model = NULL;
	CLoadedModelInfo* m_pWallModel = NULL; //벽
	CLoadedModelInfo* m_pceilingModel = NULL; //지붕
	CLoadedModelInfo* m_pfloorModel = NULL; //바닥
	CLoadedModelInfo* m_pTree01Model = NULL; //나무
	CLoadedModelInfo* m_pGrass01Model = NULL; //풀
	CLoadedModelInfo* m_pGrass02Model = NULL; //풀

	//오브젝트 계열별로 풀링 값을 따로 갖고있음
	CObjectPool<CWoodManObject> m_WoodManObjectPool;
	CObjectPool<CCornObject>           m_CornObjectPool;
	CObjectPool<CPotatoObject>         m_PotatoObjectPool;
	CObjectPool<CPumpkinObject>        m_PumpkinObjectPool;
	CObjectPool<CTomatoObject>         m_TomatoObjectPool;
	CObjectPool<CLogPlnaterObject>     m_LogPlnaterObjectPool;
	CObjectPool<CBarnObject>           m_BarnObjectPool;
	CObjectPool<CRockObject>			m_RockObjectPool;
	CObjectPool<CWallObject>			m_WallObjectPool;
	CObjectPool<CCeilingObject>			m_CeilingObjectPool;
	CObjectPool<CFloorObject>			m_FloorObjectPool;
	CObjectPool<CTreeObject>				m_TreeObjectPool;
	CObjectPool<CGrassObject>			m_GrassObjectPool;
	

	virtual void SetPlayer();
	virtual void SetNetWorkPlayer() ;


	

	//설치할 오브젝트를 클릭한 상태
	void SelectObject(CLoadedModelInfo* pMdel);
	void SelectObjects();
	void BuildingObjectCheck(); // AnimateObject에서 BuildingObject가 활성화 되어있을때 Imgui에서 변경 체크
	void BuildingObjectAnimateUpdate(float fTimeElapsed);
	void PlaceObject();

	//매번 설치를 결정할 오브젝트의 상태를 보여주는
	CBuildingObject* m_pBuildingObject = NULL;

	XMFLOAT3 RayToPlanCollision(XMFLOAT3 O, XMFLOAT3 N, XMFLOAT3 D , float& distance);
	XMFLOAT3 CMainScene::RayToTerrainCollision(XMFLOAT3 O, XMFLOAT3 D,CHeightMapTerrain* pTerrain,
		float& distance);
	void ResolvePlayerCollision(); //

	std::vector<UINT> m_vpCollisionIndex;
	int m_nCollisionCount = 1;
	
	RECT m_nWorldBounds[4];
	RECT m_nCurrentWorldBounds;
	void SetWorldLevel(int nindex); //월드에 단계를 지정 (봄 : 0, 여름 : 1, 가을 : 2, 겨울 : 3)

	CComputeShader* m_pComputeShader; // 컴퓨트 파이프라인 확장에 사용할 Shader

	virtual void CreateSSAOKernel();

	CTexture* m_pSSAONoiseTexture;
	void CreateNoiseTexture(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList);

	void UpdateCropState(); //현재 배치된 모든 Crop들에 상태를 읽고 갱신해준다 ==> 상태는 서버에서 보내주지만 여기선 모델의
	//형태를 변형시키는 것.

	void CropCheckWheather(float fTimeElapsed); //작물이 현재 날씨에 영향을 받는지 안받는지 --> 작물 기준 위쪽방향으로 ray를 쏘았을때 지붕이 있으면 세이프 없으면 out

	WeatherBlock* m_pParticleBlock = NULL;
	public:

		void PlayerTriggerObjectUpdate(); //Player에 TriggerBox에 들어온거 순서 상관없이 항상 확인되면 ok
		void PlayerTriggerAction(); //상호작용하고 있는 오브젝트에 따라 동작을 달리 조절해주는 함수

		void TriggerObjectDestory(); //상호작용이 완료되고 사라져야하는 오브젝트들을 총괄적으로 삭제해주는 함수
		//==> Spawn 함수가 Scene에 종속되어있어서 이렇게 확장

		StageSpawnData m_SpawnData01;
		StageSpawnData m_SpawnData02;
		StageSpawnData m_SpawnData03;
		StageSpawnData m_SpawnData04;
		StageSpawnData m_SpawnData05;

		StageSpawnData LoadStageResourceFromFile(Protocol::S_WORLD_INIT& pkt);
		void  SpawnStageFromData(const StageSpawnData& data);
		void ApplyPendingWorldInit();
		EObjectModelType StringToModelType(int name);
		CLoadedModelInfo* GetModelInfoByType(EObjectModelType type);

		//void ResourcePositionSettings(); //나무, 바위, 풀 설치 하는거 

		float m_fPlayerPositionX = 0.0f;
		float m_fPlayerPositionZ = 0.0f;

	public:
		CGameObject* CMainScene::FindObjectByServerID(uint64 id);
		void CMainScene::SendPlaceReq(int id,EObjectModelType type, const XMFLOAT3& pos, XMFLOAT4& qpos);

		void SendWorldInitToServer(const std::vector<WorldSpawnInfo>& list);

		void CMainScene::ParseMultiModelBlock(ifstream& InFile,StageSpawnData& data,const char* endTag);

		void CMainScene::ParseSingleModelBlock(ifstream& InFile, StageSpawnData& data, const char* endTag);
		vector<WorldSpawnInfo> toSend;
		float m_moveSendAcc = 0.0f;
		bool  m_moveDirty = false;
		XMFLOAT3 m_lastSentPos = { 0,0,0 }; // 선택(변화 감지용)
		Protocol::PlayerState m_lastSentState = Protocol::PlayerState::IDLE;
		void QueueRemoteMovementSnapshot(uint64 playerId, const RemoteMovementSnapshot& snapshot);
		void UpdateRemoteMovementInterpolation();
		std::mutex m_remoteMovementMutex;
		std::unordered_map<uint64, std::deque<RemoteMovementSnapshot>> m_remoteMovementSnapshots;
		int GUIOnOFF = 0; // 설치 아이템 설정을 보는 GUI를 안보이게 했다 보이게 했다 하는 부분
	





};
