#include "stdafx.h"
#include "MainScene.h"


static std::atomic<uint64> gTempIdGen = 1000;
bool g_hasPendingWorldInit;
Protocol::S_WORLD_INIT g_pendingWorldInit;
static uint64 MakeTempId()
{
	return gTempIdGen.fetch_add(1);
}

void CMainScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pComputeShader = new CComputeShader();
	m_cpd3dDevice = pd3dDevice;
	m_cpd3dCommandList = pd3dCommandList;

	m_pd3dRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneRootSignature;
	m_cpd3dComputeRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneComputeRootSignature;
	m_pDescriptorHeap = new CDescriptorHeap();
	//플레이어와 카메라는 해당 Heap에 저장하지 않고 바로 바인딩하고 있다 그래서
	// 여기 heap 크기 공간에 할당하지 않아도 된다

	m_nWorldBounds[0] = { -62,  -62,   62,   62 };
	m_nWorldBounds[1] = { -125, -125,  125,  125 };
	m_nWorldBounds[2] = { -187, -187,  187,  187 };
	m_nWorldBounds[3] = { -250, -250,  250,  250 };

	std::vector<const char*> modelFiles = {
	"Model/Woodman.bin",
	"Model/WorkMan.bin",

   // Corn (Crops)
   "Model/Crops/Corn_01.bin",
   "Model/Crops/Corn_02.bin",
   "Model/Crops/Corn_03.bin",
   "Model/Crops/Corn_04.bin",
   "Model/Crops/CornSeed_01.bin",

   // Potato (Crops)
 "Model/Crops/Potato_01.bin",
 "Model/Crops/Potato_02.bin",
 "Model/Crops/Potato_03.bin",
 "Model/Crops/Potato_04.bin",
 "Model/Crops/PotatoSeed_01.bin",

 // Pumpkin (Crops)
"Model/Crops/Pumpkin_01.bin",
"Model/Crops/Pumpkin_02.bin",
"Model/Crops/Pumpkin_03.bin",
"Model/Crops/Pumpkin_04.bin",
"Model/Crops/PumpkinSeed_01.bin",

// Tomato (Crops)
"Model/Crops/Tomato_01.bin",
"Model/Crops/Tomato_02.bin",
"Model/Crops/Tomato_03.bin",
"Model/Crops/Tomato_04.bin",
"Model/Crops/TomatoSeed_01.bin",

//Tree, Rock, Grass (Resource)
"Model/Object/P_Rock_01.bin",
"Model/Object/P_Rock_02.bin",
"Model/Object/SM_group22-27_LOD0.bin",
"Model/Object/SM_plant14_LOD0.bin",
"Model/Object/DreamTree.bin",

//Others
"Model/Crops/LogPlanter1Low_1x1m_01.bin",
"Model/Object/Barn.bin",
"Model/Object/ceiling.bin",
"Model/Object/floor.bin",
"Model/Object/wall.bin",

	};

	int nTexturesCountTotal = CountTexturesFromFiles(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), modelFiles) ;

	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, nTexturesCountTotal + 1 + 2 + 3 + 1 + 1 + 5 + 18 + 2 + 5 + 1 + 3 + 1 +  2 + 5 + 1, 5 + 1); //nObjects  , SkyBox(1), Plan(1), RenderTarget, ShadowMap, TerrainMap
	

	//IBL(3)

	//DXGI_FORMAT pdxgiResourceFormats[1] = { DXGI_FORMAT_R16G16B16A16_FLOAT };
	//D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	////d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * m_nSwapChainBuffers);
	//d3dRtvCPUDescriptorHandle.ptr += (::gnRtvDescriptorIncrementSize * 2);
	//CreateShaderResourceViewRtvs(pd3dDevice, pd3dCommandList, 1, pdxgiResourceFormats , d3dRtvCPUDescriptorHandle);

	InitLights();

	//CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//공용 Shader를 준비하는 과정 여기엔 CDefaultShader, CAnimatedShader 이 있다.
	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get());

	m_pSkyBox = new CSkyBoxObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), L"SkyBox/pureSky01EnvHDR.dds");
	m_pSkyBoxCloud = new CSkyBoxObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), L"SkyBox/CloudSkyEnvHDR.dds");
	//m_pPlan = new CPlanObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get());
	//m_pPlan->SetPosition(0,0,0);
	//m_pPlan->SetScale(2, 2, 2);

	int heightmapVerts = 513;
	int cells = 513;

	// 월드 크기 1000 x 1000 기준
	float worldSize = 500.0f;

	XMFLOAT3 scale;
	scale.x = worldSize / cells;
	scale.z = worldSize / cells;
	scale.y = 2.0f;       

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get() , _T("Model/HeightMapTexture/terrain.raw")
	, 513, 513, 513, 513, scale);
	m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);

	m_ppHierarchicalGameObjects.resize(250);

	m_pWoodManModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice,
		pd3dCommandList, m_pd3dRootSignature.Get(), "Model/Woodman.bin", NULL, EObjectModelType::WoodMan);

	m_pWorkManModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice,
		pd3dCommandList, m_pd3dRootSignature.Get(), "Model/WorkManani.bin", NULL, EObjectModelType::WorkMan);

	//// Corn (Crops)
	m_pCorn01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Corn_01.bin", NULL, EObjectModelType::Corn01);
	m_pCorn02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Corn_02.bin", NULL, EObjectModelType::Corn02);
	m_pCorn03Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Corn_03.bin", NULL, EObjectModelType::Corn03);
	m_pCorn04Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Corn_04.bin", NULL, EObjectModelType::Corn04);
	m_pCornSeedModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/CornSeed_01.bin", NULL, EObjectModelType::CornSeed);


	// Potato (Crops)
	m_pPotato01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Potato_01.bin", NULL, EObjectModelType::Potato01);
	m_pPotato02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Potato_02.bin", NULL, EObjectModelType::Potato02);
	m_pPotato03Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Potato_03.bin", NULL, EObjectModelType::Potato03);
	m_pPotato04Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Potato_04.bin", NULL, EObjectModelType::Potato04);
	m_pPotatoSeedModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/PotatoSeed_01.bin", NULL, EObjectModelType::PotatoSeed);


	// Pumpkin (Crops)
	m_pPumpkin01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Pumpkin_01.bin", NULL, EObjectModelType::Pumpkin01);
	m_pPumpkin02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Pumpkin_02.bin", NULL, EObjectModelType::Pumpkin02);
	m_pPumpkin03Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Pumpkin_03.bin", NULL, EObjectModelType::Pumpkin03);
	m_pPumpkin04Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Pumpkin_04.bin", NULL, EObjectModelType::Pumpkin04);
	m_pPumpkinSeedModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/PumpkinSeed_01.bin", NULL, EObjectModelType::PumpkinSeed);


	// Tomato (Crops)
	m_pTomato01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Tomato_01.bin", NULL, EObjectModelType::Tomato01);
	m_pTomato02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Tomato_02.bin", NULL, EObjectModelType::Tomato02);
	m_pTomato03Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Tomato_03.bin", NULL, EObjectModelType::Tomato03);
	m_pTomato04Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/Tomato_04.bin", NULL, EObjectModelType::Tomato04);
	m_pTomatoSeedModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/TomatoSeed_01.bin", NULL, EObjectModelType::TomatoSeed);

	// Others
	m_pLogPlanterModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Crops/LogPlanter1Low_1x1m_01.bin", NULL, EObjectModelType::LogPlanter);

	m_pBarnModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/Barn.bin", NULL, EObjectModelType::Barn);

	m_pRock01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/P_Rock_01.bin", NULL, EObjectModelType::Rock01);

	m_pRock02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/P_Rock_02.bin", NULL, EObjectModelType::Rock02);

	// Building Parts
	m_pWallModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/wall.bin", NULL, EObjectModelType::Wall);

	m_pceilingModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/ceiling.bin", NULL, EObjectModelType::Ceiling);

	m_pfloorModel = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/floor.bin", NULL, EObjectModelType::Floor);

	m_pTree01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/DreamTree.bin", NULL, EObjectModelType::Tree01);

	m_pGrass01Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/SM_plant14_LOD0.bin", NULL, EObjectModelType::Grass01);

	m_pGrass02Model = CGameObject::LoadGeometryAndAnimationFromFile(
		pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(),
		"Model/Object/SM_group22-27_LOD0.bin", NULL, EObjectModelType::Grass02);

	m_pBuildingObject = new CBuildingObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get());
	m_pBuildingObject->Activate(false);

	CFarmPlayer* pFarmPlayer = new CFarmPlayer(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), m_pWoodManModel, m_pTerrain,2 );
	
	m_pPlayer = pFarmPlayer;
	m_pPlayer->SetTag(EObjectTag::Player);
	m_pPlayer->SetID(GMyServerPlayerId); 
    // With this updated line:  
    m_pPlayer->setNetworkClient(m_upClientNetwork.get());
	m_pCamera = m_pPlayer->GetCamera();

	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0);
	enterGamePkt.set_gamestart(true);
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	m_upClientNetwork->Broadcast(sendBuffer);
	
	//플레이어를 동기화 시킬때 이렇게 함수를 통해서 호출하거나 다른 데서 호출하는 식으로? 필요할 듯하다
	SetPlayer();
	SetNetWorkPlayer();

	std::cout << "메인 Scene의 Build가 실행되었습니다." << endl;

	//SpawnAnimateObject(m_WoodManObjectPool, m_pWoodManModel, EObjectTag::Player, m_nIDPool++, 2);
	SpawnDefaultObject(m_BarnObjectPool, m_pBarnModel, EObjectTag::Environment, m_nIDPool++);


	//집
	m_ppHierarchicalGameObjects[0]->SetScale(1, 1, 1);
	m_ppHierarchicalGameObjects[0]->SetPosition(0, 0, 0);

	ApplyPendingWorldInit();
	//m_SpawnData01 = LoadStageResourceFromFile("Model/Spawn/Stage01.txt");
	//m_SpawnData02 = LoadStageResourceFromFile("Model/Spawn/Stage02.txt");
	//m_SpawnData03 = LoadStageResourceFromFile("Model/Spawn/Stage03.txt");
	//SpawnStageFromData(m_SpawnData01);
	//SpawnStageFromData(m_SpawnData02);
	//SpawnStageFromData(m_SpawnData03);


	//후처리를 위한 오브젝트
	m_pPostProcessingObject = new CPostProcessingObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get());

	//파티클 친구들
	m_pRainParticle = new CRainParticleObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), 10000);
	m_pRainParticle->Activate(false);
	m_pRainParticle->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	m_pSnowParticle = new CSnowParticleObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), 10000);
	m_pSnowParticle->Activate(false);
	m_pSnowParticle->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	m_pDustParticle = new CDustParticleObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), 100);
	m_pDustParticle->Activate(false);
	m_pDustParticle->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	m_pPunchParticleObject = new CPunchParticleObject(pd3dDevice, pd3dCommandList, m_pd3dRootSignature.Get(), 30);
	m_pPunchParticleObject->Activate(false);
	m_pPunchParticleObject->SetPosition(XMFLOAT3(0.0f, 1.0f, 0.0f));

	m_pPlayer->SetParticleObject(m_pPunchParticleObject); //플레이어가 사용할 파티클이니까 연동한다
	//Burn
	m_pParticleBlock = new WeatherBlock[MAX_BLOCK];
	m_pParticleBlock[0].center = XMFLOAT3(
		0.0f,   // X center
		5.0f,   // Y center 
		0.0f   // Z center
	);
	m_pParticleBlock[0].extent = XMFLOAT3(
		7.5f,   // X half-size
		5.5f,   // Y half-size
		15.5f  // Z half-size
	);

}

void CMainScene::SetPlayer()
{

}

void CMainScene::SetNetWorkPlayer()
{

}

void CMainScene::InitLights()
{
	//DirectionLight(1) , SpotLight(8), PointLight(0) 순서 지켜야한다
	m_nLights = 57;
	m_pLights = new Light[m_nLights];

	m_pLightData = new LightingData[m_nLights - 1]; //DirectionLight를 뺸

	::ZeroMemory(m_pLights, sizeof(Light) * m_nLights);
	::ZeroMemory(m_pLightData, sizeof(LightingData) * (m_nLights-1));

	// 월드에 존재하는 라이트 컨트롤러
	m_vectorLightControl.clear();
	m_vectorLightControl.resize(m_nLights);

	// Directional Light (index 0)

	m_vectorLightControl[0].SetLight(
		LIGHT_DIRECTIONAL,
		XMFLOAT3(1.0f + 1.0f, 0.95f + 1.0f, 0.85f + 1.0f),
		Vector3::Normalize(XMFLOAT3(0.3f, -0.5f, -0.2f)),
		XMFLOAT3(-48.8f, 83.3f, 25.9f),
		1
	);

	m_vectorLightControl[0].SetShadowUse(1);

	// Spot Light 1

	m_vectorLightControl[1].SetLight(
		LIGHT_SPOT,
		XMFLOAT3(5.0f, 5.0f, 5.0f),
		Vector3::Normalize(XMFLOAT3(0, -1, 0)),
		XMFLOAT3(0, 3, 12),
		0.0f,
		20.0f,
		5.0f,
		1,
		1
	);

	// Spot Light 2 ==> 이거랑 
	m_vectorLightControl[2].SetLight(
		LIGHT_SPOT,
		XMFLOAT3(5.0f, 5.0f, 5.0f),
		Vector3::Normalize(XMFLOAT3(0, -1, 0)),
		XMFLOAT3(0, 3, 6),
		0.0f,
		20.0f,
		5.0f,
		1,
		1
	);

	// Spot Light 4
	m_vectorLightControl[3].SetLight(
		LIGHT_SPOT,
		XMFLOAT3(5.0f, 5.0f, 5.0f),
		Vector3::Normalize(XMFLOAT3(0, -1, 0)),
		XMFLOAT3(0, 3, -6),
		0.0f,
		20.0f,
		5.0f,
		1,
		1
	);

	// Spot Light 3 ==이거가 문제가있어 깊이맵이 이상한데서 만들어지고 있다
	m_vectorLightControl[4].SetLight(
		LIGHT_SPOT,
		XMFLOAT3(5.0f, 5.0f, 5.0f),
		Vector3::Normalize(XMFLOAT3(0, -1, 0)),
		XMFLOAT3(0, 3, -12),
		0.0f,
		20.0f,
		5.0f,
		1,
		1
	);

	int lightIndex = 5; // 0~4는 이미 사용 중

	float xPositions[] = { -30.0f, -15.0f, 15.0f, 30.0f };
	float zPositions[] = { -60.0f ,-50.0f, -40.0f, -30.0f, -20.0f, -10.0f, 0.0f,
							 10.0f, 20.0f, 30.0f, 40.0f , 50.0f, 60.0f};

	for (int z = 0; z < _countof(zPositions); ++z)
	{
		for (int x = 0; x < _countof(xPositions); ++x)
		{

			m_vectorLightControl[lightIndex].SetLight(
				LIGHT_SPOT,
				XMFLOAT3(1.0f, 1.0f, 1.0f),        // color
				XMFLOAT3(0, -1, 0),               // direction
				XMFLOAT3(xPositions[x], 4.0f, zPositions[z]), // position
				0.0f,     // inner angle
				7.0f,     // outer angle
				5.0f,     // radius
				1,        // enable
				0         // shadow off (성능 테스트용!)
			);

			lightIndex++;
		}
	}

}

bool CMainScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		//마우스 캡쳐를 하고 현재 마우스 위치를 가져온다
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_RBUTTONDOWN:
	{
	
		break;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//마우스 캡쳐를 해제한다.
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}

	return false;
}

bool CMainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_SPACE:
		{
			if (m_pPlayer)
			{
				CFarmPlayer* player = (CFarmPlayer*)m_pPlayer;
				player->Jump();

				m_moveDirty = true; // 20Hz 루프에도 반영되게
				SendObjectMoveToServer(m_pPlayer, Protocol::PlayerState::WALK); // JUMP 없으면 IDLE/RUN로

			}
			break;
		}
		default:
		{
			break;
		}
		}
		break;
	}
	case WM_KEYUP:
		switch (wParam)
		{
			/*‘F1’ 키를 누르면 1인칭 카메라, ‘F2’ 키를 누르면 스페이스-쉽 카메라로 변경한다, ‘F3’ 키를 누르면 3인칭 카메라
로 변경한다.*/
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer)
			{
				m_pCamera = m_pPlayer->ChangeCamera((wParam - VK_F1 + 1),
					m_pGameTimer->GetTimeElapsed());
			}
			break;
		/*case '[' :
			CSceneManager::GetInstance().PopScene();
			break;*/


		case 'Z': //애니메이션 상태 전환 임시 테스트
		{
			//배열상 가장 앞에 있는 친구를 삭제
			if (m_ppHierarchicalGameObjects[0])
			{
				DeSpawnAnimateObject(m_AnimatedObjectPool,m_ppHierarchicalGameObjects[0]);

				for (int i = 0; i < m_ppHierarchicalGameObjects.size(); i++)
				{
					if (m_ppHierarchicalGameObjects[i] == NULL)
					{
						std::cout << i << "몇 번째 값까지 있다" << endl;
						break;
					}
				}
			}
	
			break;
		}

		case 'X':
		{
			SpawnAnimateObject(m_AnimatedObjectPool,m_pWorkManModel, EObjectTag::Player, m_nIDPool++, 2);
			std::cout << m_nHierarchicalGameObjects <<"총 추가된 오브젝트 갯수" << endl;
			
			break;
		}

		case 'Q':
		{
			m_pPlayer->PrintMatrixWorld();
			m_pPlayer->PrintMatrixToParent();
			break;
		}
		
		case 'J' :
		{
			m_pPunchParticleObject->PrintStartTIme();
			//m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			break;
		}




		//임시 지정한 빌딩 활성화 버튼 B와 N
		case'B':
		{
			//std::cout << "키가 여러번 먹히나?" << endl;
			//SelectObject(m_pLogPlanterModel);
			//SelectObject(m_pfloorModel);
			if (GUIOnOFF == 0)
			{
				GUIOnOFF = 1;
			}
			else
			{
				GUIOnOFF = 0;
			}
			SelectObjects(); // 한 번 누르면 켜지고 두 번 누르면 꺼집니다.
			break;
		}

		case'L':
		{
		/*	m_pPlayer->PrintMatrixWorld();
			m_pPlayer->PrintMatrixToParent();
			m_pPlayer->PrintOBB();*/

			/*for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->GetActivie())
				{
					m_ppHierarchicalGameObjects[i]->PrintMatrixWorld();

				}
			}*/

			static float testa = 0.0f;
			CCropObject* pCrop =
				static_cast<CCropObject*>(m_ppHierarchicalGameObjects[4]);
			pCrop->SetGrowth(testa += 10);
			UpdateCropState();

			/*if (m_pBuildingObject)
			{
				m_pBuildingObject->PrintMatrixWorld();
			}*/
			break;
		}
		case'C':
		{
			//PrintLightPositions(m_pLights, 4);
			float h = m_pTerrain->GetHeight(m_pPlayer->GetPosition().x + 249, m_pPlayer->GetPosition().z + 249);
			std::cout << "현재 위치 기준 Terrain의 높이" << h << endl;
			break;
		}
		case 'F':
		{
			PlaceObject(); //오브젝트 배치
			if (!m_pBuildingObject->GetActivie()) //빌드 모드가 아니라면
			{
				PlayerTriggerAction();
				
			}
			break;
		}
		default:
		{
			break;
		}
		}
		break;
	default:
		break;
	}

	return false;
}

void CMainScene::PreviousTransform()
{
	m_pPlayer->SetPreviousPosition(m_pPlayer->GetPosition());
	//m_pPlayer->m_bOnBuilding = false;
}

bool CMainScene::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;
	/*키보드의 상태 정보를 반환한다. 화살표 키(‘→’, ‘←’, ‘↑’, ‘↓’)를 누르면 플레이어를 오른쪽/왼쪽(로컬 x-축), 앞/
   뒤(로컬 z-축)로 이동한다. ‘Page Up’과 ‘Page Down’ 키를 누르면 플레이어를 위/아래(로컬 y-축)로 이동한다.*/
	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
	}
	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	/*마우스를 캡쳐했으면 마우스가 얼마만큼 이동하였는 가를 계산한다. 마우스 왼쪽 또는 오른쪽 버튼이 눌러질 때의
   메시지(WM_LBUTTONDOWN, WM_RBUTTONDOWN)를 처리할 때 마우스를 캡쳐하였다. 그러므로 마우스가 캡쳐된
   것은 마우스 버튼이 눌려진 상태를 의미한다. 마우스 버튼이 눌려진 상태에서 마우스를 좌우 또는 상하로 움직이면 플
   레이어를 x-축 또는 y-축으로 회전한다.*/
	if (::GetCapture() == m_hWnd)
	{

		//마우스 커서를 화면에서 없앤다(보이지 않게 한다).
		::SetCursor(NULL);
		//현재 마우스 커서의 위치를 가져온다.
		::GetCursorPos(&ptCursorPos);
		//마우스 버튼이 눌린 상태에서 마우스가 움직인 양을 구한다.
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		//마우스 커서의 위치를 마우스가 눌려졌던 위치로 설정한다.
		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}
	//마우스 또는 키 입력이 있으면 플레이어를 이동하거나(dwDirection) 회전한다(cxDelta 또는 cyDelta).
	if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		if (cxDelta || cyDelta)
		{
			/*cxDelta는 y-축의 회전을 나타내고 cyDelta는 x-축의 회전을 나타낸다. 오른쪽 마우스 버튼이 눌려진 경우
		   cxDelta는 z-축의 회전을 나타낸다.*/
			if (pKeyBuffer[VK_RBUTTON] & 0xF0)
				m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
			else
				m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
		}
	}
	
	/*플레이어를 dwDirection 방향으로 이동한다(실제로는 속도 벡터를 변경한다). 이동 거리는 시간에 비례하도록 한다.
	플레이어의 이동 속력은 (50/초)로 가정한다.*/
	Protocol::PlayerState newState = Protocol::PlayerState::IDLE;
	if (dwDirection)
	{
		m_pPlayer->Move(dwDirection, 249.0f * m_pGameTimer->GetTimeElapsed(), true);
		if (m_pPlayer->m_pSkinnedAnimationController)
		{
			m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, false);
			m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationClip(0, 1);
		}
		newState = Protocol::PlayerState::WALK;
	}
	else
	{
		m_pPlayer->Move(0, 249.0f*m_pGameTimer->GetTimeElapsed(), true);

		newState = Protocol::PlayerState::IDLE;
	}
	XMFLOAT3 currentInput = m_pPlayer->m_xmf3InputDir;
	const float inputLength = std::sqrt(currentInput.x * currentInput.x + currentInput.z * currentInput.z);
	if (inputLength > 0.001f)
	{
		currentInput.x /= inputLength;
		currentInput.z /= inputLength;
	}
	else
	{
		currentInput.x = 0.0f;
		currentInput.z = 0.0f;
	}
	if (std::fabs(currentInput.x - m_lastSentInput.x) > 0.01f ||
		std::fabs(currentInput.z - m_lastSentInput.z) > 0.01f)
	{
		m_moveDirty = true;
	}
	if (newState != m_lastSentState)
	{
		m_lastSentState = newState;
		m_moveDirty = true;
	}

	//플레이어를 실제로 이동하고 카메라를 갱신한다. 중력과 마찰력의 영향을 속도 벡터에 적용한다. ==> Update는 따로 빼야할 것 같지만 Input 안에 넣어둔 이유는 입력이후에 update하는 것을 보다
	//직관적으로 하기 위해서
	m_pPlayer->Update(m_pGameTimer->GetTimeElapsed());


	return false;
}

void CMainScene::ReleaseObjects()
{
	if (m_pd3dRootSignature) m_pd3dRootSignature.Reset();	//ComPtr을 썻기에 구지 필요하진 않은데

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

	for (CGameObject* obj : m_ppHierarchicalGameObjects)
	{
		delete obj;
	}
	m_ppHierarchicalGameObjects.clear();

	/*if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++)
		{
			delete m_ppHierarchicalGameObjects[i];
		}
		delete[] m_ppHierarchicalGameObjects;
	}*/
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();
	}
	if (m_pShaders)
	{
		delete[] m_pShaders;
	}
	if (m_pd3dPipelineState) m_pd3dPipelineState.Reset();	//항상 Release로 메모리 관리가 필요하단걸 잊지말자.
}

void CMainScene::ReleaseUploadBuffers()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j])
		{
			m_ppObjects[j]->ReleaseUploadBuffers();
		}
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseUploadBuffers();
	}
	//if (m_ppShader)
	//{
	//	for (int j = 0; j < m_nShaders; j++) if (m_ppShader[j])
	//		m_ppShader[j]->ReleaseUploadBuffers();
	//}
}

XMFLOAT3 CMainScene::RayToPlanCollision(XMFLOAT3 O, XMFLOAT3 N, XMFLOAT3 D , float& distance)
{
	//XMFLOAT3 x = XMFLOAT3(0, 0, 0);
	//x = Vector3::Subtract(O, Vector3::ScalarProduct(D, Vector3::DotProduct(N, O) / Vector3::DotProduct(N, D)));
	//return x;

	 // XMVECTOR로 변환
	XMVECTOR vO = XMLoadFloat3(&O);
	XMVECTOR vN = XMLoadFloat3(&N);
	XMVECTOR vD = XMLoadFloat3(&D);

	// dot(N, O)
	float NO = XMVectorGetX(XMVector3Dot(vN, vO));
	// dot(N, D)
	float ND = XMVectorGetX(XMVector3Dot(vN, vD));

	// t = -(dot(N, O) / dot(N, D))
	float t = NO / ND;

	distance = fabsf(t);

	// X = O - D * t
	XMVECTOR vX = XMVectorSubtract(vO, XMVectorScale(vD, t));

	// 다시 float3로 저장
	XMFLOAT3 X;
	XMStoreFloat3(&X, vX);

	return X;
}

XMFLOAT3 CMainScene::RayToTerrainCollision(
	XMFLOAT3 O,
	XMFLOAT3 D,
	CHeightMapTerrain* pTerrain,
	float& distance)
{
	XMFLOAT3 dir = Vector3::Normalize(D);

	const float maxDist = 10.0f;   // 네가 말한 그대로
	const float step = 0.5f;    // 적당히 (원하면 1.0f)

	float t = 0.0f;

	while (t <= maxDist)
	{
		XMFLOAT3 p = Vector3::Add(O, Vector3::ScalarProduct(dir, t));

		// Player랑 동일한 HeightMap 좌표 보정
		XMFLOAT3 queryPos = Vector3::Add(p, XMFLOAT3(249.0f, 0.0f, 249.0f));
		float terrainHeight = pTerrain->GetHeight(queryPos.x, queryPos.z);

		if (p.y <= terrainHeight)
		{
			p.y = terrainHeight;
			distance = t;
			return p;
		}

		t += step;
	}

	// 못 맞췄으면 Ray 끝 위치 반환
	distance = maxDist;
	return Vector3::Add(O, Vector3::ScalarProduct(dir, maxDist));
}


void CMainScene::ResolvePlayerCollision()
{
	/*
	* 여기 충돌 정리한거 나중에 한 번 복습 필요==> 복습하고 정리해서 개선사항도 더 생각해보자
	*/
	if (!m_pPlayer) return;

	//따지고보면 바로 아래 for문은 OnCollisionExit이거 그 아래는 OnCollisionStay라고 봐야한다
	//충돌을 그만뒀을 경우에 대하여 처리하는 반복문이지만 현재는 임시방편으로 무언가 올라갔을 경우에만 저장하는 상태
	for (int i = 0; i < m_nCollisionCount; i++)
	{
		if (m_vpCollisionIndex[i] == -1)
		{
			break;
		}
		if (!m_pPlayer->m_obbBox.Intersects(m_ppHierarchicalGameObjects[m_vpCollisionIndex[i]]->m_obbBox))
		{
			m_vpCollisionIndex[i] = -1;
			if (m_pPlayer->m_bOnBuilding)
			{
				m_pPlayer->m_bOnBuilding = false;
			}
		}
	}
	// 이전 / 현재 위치
	XMVECTOR vPrevPos = XMLoadFloat3(&m_pPlayer->GetPreviousPosition());
	XMVECTOR vCurrPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	// 실제 이동 벡터
	XMVECTOR vMove = XMVectorSubtract(vCurrPos, vPrevPos);

	float moveLenSq = XMVectorGetX(XMVector3LengthSq(vMove));
	if (moveLenSq < 1e-6f) return; // 이동 거의 없음

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (!m_ppHierarchicalGameObjects[i])
		{
			continue;
		}
		CGameObject* obj = m_ppHierarchicalGameObjects[i];
		if (!obj || !obj->m_bIsBoxCollision) continue;

		if (!m_pPlayer->m_obbBox.Intersects(obj->m_obbBox))
			continue;

		// 플레이어 중심
		XMVECTOR vPlayerCenter = XMLoadFloat3(&m_pPlayer->m_obbBox.Center);
		// 상대 OBB 기준 최근접 표면점
		XMVECTOR vClosest = ClosetPointOnOBB(obj->m_obbBox, vPlayerCenter);

		//vPlayerCenter = XMVectorSetY(vPlayerCenter, XMVectorGetY(vClosest));
		
		float moveY = XMVectorGetY(vMove);

		// 낙하 중이 아닐 때만 Y 평면화
		if (moveY >= -0.001f)   // 수평 이동 또는 상승
		{
			vPlayerCenter = XMVectorSetY(vPlayerCenter, XMVectorGetY(vClosest));
		}

		// 침투 방향
		XMVECTOR vNormal = XMVectorSubtract(vPlayerCenter, vClosest);
		// 침투 방향 (OBB 표면 > 플레이어)
		//XMVECTOR vNormal = XMVectorSubtract(vPlayerCenter, vClosest);

		float nLenSq = XMVectorGetX(XMVector3LengthSq(vNormal));
		if (nLenSq < 1e-6f) continue;

		vNormal = XMVector3Normalize(vNormal);

		// 이동 방향과 법선의 내적
		float dotMoveNormal = XMVectorGetX(XMVector3Dot(vMove, vNormal));

		// 이미 멀어지는 중이면 무시
		if (dotMoveNormal >= 0.0f)
			continue;

		float normalY = XMVectorGetY(vNormal);

		//45도 이상 넘어간 경우
		if (normalY > 0.707f)
		{
			//m_pPlayer->m_bOnGround = true;
			m_pPlayer->m_bOnBuilding = true;

			// Y축 이동 제거 (XZ만 유지)
			XMVECTOR vMoveNoY = XMVectorSet(
				XMVectorGetX(vMove),
				0.0f,
				XMVectorGetZ(vMove),
				0.0f
			);

			XMVECTOR vNewPos = XMVectorAdd(vPrevPos, vMoveNoY);

			XMFLOAT3 newPos;
			XMStoreFloat3(&newPos, vNewPos);
			m_pPlayer->SetPosition(newPos);

			m_vpCollisionIndex[0] = i; //해당 오브젝트를 직접 참조하는 구조가 아닌 인덱스를 참조하는 구조
			continue; // 슬라이딩 처리 안 함
		}



		// 법선 방향 성분 제거 (슬라이딩)
		XMVECTOR vCorrection = XMVectorScale(vNormal, dotMoveNormal);
		XMVECTOR vNewMove = XMVectorSubtract(vMove, vCorrection);

		// 새로운 위치 = 이전 위치 + 수정된 이동
		XMVECTOR vNewPos = XMVectorAdd(vPrevPos, vNewMove);

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, vNewPos);
		m_pPlayer->SetPosition(newPos);

		// 다음 오브젝트와의 충돌은 새 위치 기준
		vCurrPos = vNewPos;
		vMove = XMVectorSubtract(vCurrPos, vPrevPos);
	}
}

void CMainScene::SetWorldLevel(int nindex)
{
	switch (nindex) //음 나무,
	{
		case 0:
		{
			
			break;
		}
		case 1:
		{

			break;
		}
		case 2:
		{

			break;
		}
		case 3:
		{

			break;
		}
		default:
		{

			break;
		}
	}
	m_nCurrentWorldBounds = m_nWorldBounds[nindex];
	m_pPlayer->SetMovementBound(m_nCurrentWorldBounds);
}

void CMainScene::ApplyWorldState(uint32 season, uint32 weather, uint32 missionScore)
{
	if (season > 3 || weather > 4)
	{
		std::cout << "[WORLD_STATE] invalid season=" << season
			<< " weather=" << weather << std::endl;
		return;
	}

	m_nSeasonIndex = static_cast<int>(season);
	m_nWeather = static_cast<int>(weather);
	m_fScore = static_cast<float>(missionScore);

	if (m_pTerrain != nullptr)
		m_pTerrain->SetSeason(m_nSeasonIndex);
}

void CMainScene::CreateSSAOKernel()
{
	// Map 된 CB 메모리 전체 초기화
	memset(m_pcbSSAOKernel, 0, sizeof(SSAOKernelCB));

	std::uniform_real_distribution<float> random01(0.0f, 1.0f);
	std::uniform_real_distribution<float> random11(-1.0f, 1.0f);
	std::default_random_engine generator;

	for (int i = 0; i < SSAO_KERNEL_SIZE; ++i)
	{

		XMFLOAT3 sample;
		sample.x = random11(generator);
		sample.y = random11(generator);
		sample.z = random01(generator);

		XMVECTOR v = XMLoadFloat3(&sample);
		v = XMVector3Normalize(v);

	
		float scale = static_cast<float>(i) / SSAO_KERNEL_SIZE;
		scale = 0.1f + 0.9f * scale * scale;

		v = XMVectorScale(v, scale);


		XMStoreFloat3(&m_pcbSSAOKernel->kernel[i], v);
	}
}

void CMainScene::CreateNoiseTexture(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CMainScene::UpdateCropState()
{
	
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i]->GetTag() != EObjectTag::Crop)
			continue;

		CCropObject* pCrop =
			static_cast<CCropObject*>(m_ppHierarchicalGameObjects[i]);
		
		
		float g = pCrop->GetGrowth();

		switch (pCrop->GetType())
		{
			// ================= Corn =================
		case EObjectModelType::CornSeed:
		case EObjectModelType::Corn01:
		case EObjectModelType::Corn02:
		case EObjectModelType::Corn03:
		case EObjectModelType::Corn04:
		{
			if (g >= 125.0f)
				pCrop->ChangeModel(m_pCorn04Model);   // 시듦
			else if (g >= 100.0f)
				pCrop->ChangeModel(m_pCorn03Model);   // 완성
			else if (g >= 70.0f)
				pCrop->ChangeModel(m_pCorn02Model);
			else if (g >= 25.0f)
				pCrop->ChangeModel(m_pCorn01Model);
			else
				pCrop->ChangeModel(m_pCornSeedModel);
			break;
		}
		// ================= Potato =================
		case EObjectModelType::PotatoSeed:
		case EObjectModelType::Potato01:
		case EObjectModelType::Potato02:
		case EObjectModelType::Potato03:
		case EObjectModelType::Potato04:
		{
			if (g >= 125.0f)
				pCrop->ChangeModel(m_pPotato04Model);
			else if (g >= 100.0f)
				pCrop->ChangeModel(m_pPotato03Model);
			else if (g >= 70.0f)
				pCrop->ChangeModel(m_pPotato02Model);
			else if (g >= 25.0f)
				pCrop->ChangeModel(m_pPotato01Model);
			else
				pCrop->ChangeModel(m_pPotatoSeedModel);
			break;
		}

		// ================= Pumpkin =================
		case EObjectModelType::PumpkinSeed:
		case EObjectModelType::Pumpkin01:
		case EObjectModelType::Pumpkin02:
		case EObjectModelType::Pumpkin03:
		case EObjectModelType::Pumpkin04:
		{
			if (g >= 125.0f)
				pCrop->ChangeModel(m_pPumpkin04Model);
			else if (g >= 100.0f)
				pCrop->ChangeModel(m_pPumpkin03Model);
			else if (g >= 70.0f)
				pCrop->ChangeModel(m_pPumpkin02Model);
			else if (g >= 25.0f)
				pCrop->ChangeModel(m_pPumpkin01Model);
			else
				pCrop->ChangeModel(m_pPumpkinSeedModel);
			break;
		}

		// ================= Tomato =================
		case EObjectModelType::TomatoSeed:
		case EObjectModelType::Tomato01:
		case EObjectModelType::Tomato02:
		case EObjectModelType::Tomato03:
		case EObjectModelType::Tomato04:
		{
			if (g >= 125.0f)
				pCrop->ChangeModel(m_pTomato04Model);
			else if (g >= 100.0f)
				pCrop->ChangeModel(m_pTomato03Model);
			else if (g >= 70.0f)
				pCrop->ChangeModel(m_pTomato02Model);
			else if (g >= 25.0f)
				pCrop->ChangeModel(m_pTomato01Model);
			else
				pCrop->ChangeModel(m_pTomatoSeedModel);
			break;
		}

		default:
			break;
		}
	/*	Protocol::C_CROPSTATE pkt;
		pkt.set_model_type(static_cast<google::protobuf::uint32>(pCrop->GetType()));
		pkt.set_object_id(pCrop->GetID());
		auto sendBufffer = ServerPacketHandler::MakeSendBuffer(pkt);
		m_upClientNetwork->SendToServer(sendBufffer);*/

	}
}

void CMainScene::CropCheckWheather(float fTimeElapsed)
{
	static float TimeCount = 0.0f;
	TimeCount += fTimeElapsed;

	// 1초마다 한 번만 체크 (조절 가능)
	if (TimeCount < 1.0f)
		return;

	TimeCount = 0.0f;

	// 비 / 눈만 처리
	if (m_nWeather != 1 && m_nWeather != 2)
		return;

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i]->GetTag() != EObjectTag::Crop)
			continue;

		CCropObject* pCrop =
			static_cast<CCropObject*>(m_ppHierarchicalGameObjects[i]);

		XMFLOAT3 cropPos = pCrop->GetPosition();

		// Crop 중심에서 위로 1 올린 지점
		XMFLOAT3 rayOrigin = Vector3::Add(cropPos, XMFLOAT3(0.0f, 1.0f, 0.0f));
		XMFLOAT3 rayDir = XMFLOAT3(0.0f, 1.0f, 0.0f); // 윗 방향

		XMVECTOR origin = XMLoadFloat3(&rayOrigin);
		XMVECTOR dir = XMLoadFloat3(&rayDir);

		bool bHasCeiling = false;
		float distance = 0.0f;

		for (int j = 0; j < m_nHierarchicalGameObjects; j++)
		{
			// 충돌 활성화 + Ceiling 태그만 검사
			if (m_ppHierarchicalGameObjects[j]->GetTag() == EObjectTag::Ceiling &&
				m_ppHierarchicalGameObjects[j]->m_bIsBoxCollision)
			{
			
				if (m_ppHierarchicalGameObjects[j]->m_obbBox.Intersects(origin, dir, distance))
				{
					bHasCeiling = true;
	
					break;
				}
			}
		}

		Protocol::C_SEEDGROWTH pkt;
		pkt.set_object_id(pCrop->GetID());
		// 비/눈 + 천장 없음  성장치 감소 ==> 이 부분은 임의로 넣은거 바뀔 가능성 있음 서버와 연동하면서 
		if (!bHasCeiling)
		{
			pkt.set_cropcheckcell(0);
		}
		else
		{
			pkt.set_cropcheckcell(1);
		}

		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		m_upClientNetwork->SendToServer(sendBuffer);

	}
}

void CMainScene::AnimateObjects(float fTimeElapsed)
{
	UpdateRemoteMovementInterpolation();
	UpdateCropState();
	m_fTime += fTimeElapsed;
	m_fElapsedTime = fTimeElapsed;
	CScene::AnimateObjects(fTimeElapsed);
	
	m_pTerrain->SetSeason(m_nSeasonIndex);

	static int nIndex = 0;
	if (nIndex != m_nSeasonIndex)
	{
		SetWorldLevel(m_nSeasonIndex);
		nIndex = m_nSeasonIndex;
	}


	//mjg-추가 애니메이션 땜에 했음.
	m_moveSendAcc += m_pGameTimer->GetTimeElapsed(); // 이미 쓰는 elapsed

	const bool needsHeartbeat = m_lastSentState == Protocol::PlayerState::WALK && m_moveSendAcc >= 0.5f;
	if (m_moveDirty || needsHeartbeat)
	{
		const float movePacketDelta = m_moveSendAcc;
		m_moveSendAcc = 0.0f;

		XMFLOAT3 input = m_pPlayer->m_xmf3InputDir;
		const float length = std::sqrt(input.x * input.x + input.z * input.z);
		if (length > 0.001f)
		{
			input.x /= length;
			input.z /= length;
		}
		else
		{
			input.x = 0.0f;
			input.z = 0.0f;
		}
		SendObjectMoveToServer(m_pPlayer, m_lastSentState,
			input.x, input.z, movePacketDelta);
		m_lastSentInput = input;
		m_moveDirty = false;
	}

	TriggerObjectDestory(); //오브젝트를 돌면서 확인

	//날씨에 따라 작물이 영향을 받는 정도
	CropCheckWheather(fTimeElapsed); //이 부분은 서버와 연동될 수 있는 가능성이 존재함으로 fTimeElapsed로 시간 제어중

	if (bIsFirstAnimate)
	{
		ResolvePlayerCollision();
	}

	XMFLOAT3 pos = m_pPlayer->GetPosition();

	if (m_pRainParticle && m_pRainParticle->GetActivie())
	{
		if (Vector3::Length(Vector3::Subtract(pos, m_pRainParticle->GetPosition())) > 30.0f)
		{
			m_pRainParticle->SetPosition(pos);
		}

		m_pRainParticle->SetParticleState(m_fTime, fTimeElapsed, 30.0f, 0.25f, PF_SNOW | PF_USE_NOISE, XMFLOAT3(1.0f, 1.0f, 1.0f), 1, m_pParticleBlock);
	}

	else if (m_pSnowParticle && m_pSnowParticle->GetActivie())
	{
		if (Vector3::Length(Vector3::Subtract(pos, m_pSnowParticle->GetPosition())) > 30.0f)
		{
			m_pSnowParticle->SetPosition(pos);
		}
		m_pSnowParticle->SetParticleState(m_fTime, fTimeElapsed, 30.0f, 0.55f, PF_RAIN | PF_USE_NOISE, XMFLOAT3(1.0f, 1.0f, 1.0f), 1, m_pParticleBlock);
	}

	else if (m_pDustParticle && m_pDustParticle->GetActivie())
	{
		if (Vector3::Length(Vector3::Subtract(pos, m_pDustParticle->GetPosition())) > 30.0f)
		{
			m_pDustParticle->SetPosition(pos);
		}
		m_pDustParticle->SetParticleState(m_fTime, fTimeElapsed, 30.0f, 0.1f, PF_DUST | PF_USE_NOISE | PF_USE_COLOR, XMFLOAT3(0.78f, 0.72f, 0.60f), 1, m_pParticleBlock);
	}


	if (m_pPunchParticleObject && m_pPunchParticleObject->GetActivie())
	{
		m_pPunchParticleObject->Animate(fTimeElapsed, m_fTime);
		m_pPunchParticleObject->SetParticleState(m_fTime, fTimeElapsed, 5.0f, 1.0f, PF_PUNCH | PF_USE_COLOR, XMFLOAT3(0.6f, 0.08f, 0.08f), 0, NULL);
	}

	PlayerTriggerObjectUpdate(); //플레이어와 상호작용하는 모든 오브젝트들에 대하여 플레이어한테 전달하는 것.

	//물건을 짓는 행위는 전부 여기 함수에서 처리한다
	BuildingObjectAnimateUpdate(fTimeElapsed);

	XMFLOAT3 pos2 = m_pPlayer->GetPosition();
	m_fPlayerPositionX = pos2.x;
	m_fPlayerPositionZ = pos2.z;
}

void CMainScene::QueueRemoteMovementSnapshot(uint64 playerId,
	const RemoteMovementSnapshot& snapshot)
{
	std::lock_guard<std::mutex> guard(m_remoteMovementMutex);
	auto& snapshots = m_remoteMovementSnapshots[playerId];
	if (!snapshots.empty() && snapshot.serverTick <= snapshots.back().serverTick)
		return;
	snapshots.push_back(snapshot);
	while (snapshots.size() > 8)
		snapshots.pop_front();
}

void CMainScene::UpdateRemoteMovementInterpolation()
{
	constexpr uint64 interpolationDelayMs = 100;
	const uint64 nowMs = ::GetTickCount64();
	const uint64 renderTimeMs = nowMs > interpolationDelayMs ? nowMs - interpolationDelayMs : 0;

	std::lock_guard<std::mutex> guard(m_remoteMovementMutex);
	for (auto& entry : m_remoteMovementSnapshots)
	{
		auto& snapshots = entry.second;
		while (snapshots.size() >= 2 && snapshots[1].arrivalTimeMs <= renderTimeMs)
			snapshots.pop_front();
		if (snapshots.empty())
			continue;

		const RemoteMovementSnapshot* from = &snapshots.front();
		const RemoteMovementSnapshot* to = from;
		float alpha = 1.0f;
		if (snapshots.size() >= 2)
		{
			to = &snapshots[1];
			const uint64 spanMs = to->arrivalTimeMs - from->arrivalTimeMs;
			if (spanMs > 0)
			{
				const float rawAlpha = static_cast<float>(renderTimeMs - from->arrivalTimeMs) /
					static_cast<float>(spanMs);
				alpha = rawAlpha < 0.0f ? 0.0f : (rawAlpha > 1.0f ? 1.0f : rawAlpha);
			}
		}

		CGameObject* target = FindObjectByServerID(entry.first);
		if (target == nullptr)
			continue;
		XMFLOAT3 position(
			from->position.x + (to->position.x - from->position.x) * alpha,
			from->position.y + (to->position.y - from->position.y) * alpha,
			from->position.z + (to->position.z - from->position.z) * alpha);
		if (from == to && renderTimeMs > to->arrivalTimeMs)
		{
			const float extrapolationSeconds = (std::min)(
				static_cast<float>(renderTimeMs - to->arrivalTimeMs) / 1000.0f, 0.5f);
			position.x += to->direction.x * to->moveSpeed * extrapolationSeconds;
			position.z += to->direction.z * to->moveSpeed * extrapolationSeconds;
		}
		target->SetPosition(position.x, position.y, position.z);

		XMVECTOR rotation = XMQuaternionSlerp(XMLoadFloat4(&from->rotation),
			XMLoadFloat4(&to->rotation), alpha);
		XMFLOAT4 interpolatedRotation;
		XMStoreFloat4(&interpolatedRotation, rotation);
		target->SetQuaternion(interpolatedRotation);
		target->m_nAnimation = static_cast<int>(to->state);
		if (CAnimatedObject* animated = dynamic_cast<CAnimatedObject*>(target))
			animated->SetAnimationState(static_cast<EAnimationState>(to->state));
	}
}

void CMainScene::OnlyShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pPlan)
	{
		m_pPlan->Render(pd3dCommandList, m_pCamera, true, false, m_bIsShowWire, true);
	}
	/*if (m_pTerrain)
	{
		m_pTerrain->Render(pd3dCommandList, m_pCamera, true, false, m_bIsShowWire, true);
	}*/
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->GetActivie())
		{
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) //Animation이 있는 애들은 Bone 캐시로 저장해서 한 번만 해주면 됨
			{
				m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			}
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera, true, false, m_bIsShowWire, true);
		}
	}
	if (m_pPlayer)
	{
		//m_pPlayer->Animate(m_fElapsedTime);
		m_pPlayer->ShadowRender(pd3dCommandList, m_pCamera);
	}
}



void CMainScene::PrintLightPositions(const Light* pLights, int lightCount)
{
	std::cout << "==== m_pLights Order (Position) ====" << std::endl;

	for (int i = 0; i < lightCount; ++i)
	{
		const Light& l = pLights[i];

		std::cout << "[" << i << "] ";

		if (l.bIsUse == 0)
		{
			std::cout << "Unused" << std::endl;
			continue;
		}

		std::cout << "Pos ("
			<< l.position.x << ", "
			<< l.position.y << ", "
			<< l.position.z << ")";

		// 그림자 여부도 같이 보면 좋음
		if (l.bIsShadowSet)
			std::cout << "  [Shadow]";

		std::cout << std::endl;
	}

	std::cout << "===================================" << std::endl;
}


void CMainScene::Lightsort(Light* pLights, LightingData* pLightingdata ,CCamera* pCamera , vector<LightControl> vectorLightControl)
{
	pLights[0] = vectorLightControl[0].GetLight();

	for (int i = 0; i < MAX_LIGHTS - 1; ++i)
	{
		pLightingdata[i].radius = 0.0f;
		pLightingdata[i].intensity = 0.0f;
		pLightingdata[i].Type = 0;
	}

	struct SpotCandidate
	{
		LightControl* ctrl;
		float distance;
	};

	std::vector<SpotCandidate> candidates;

	XMFLOAT3 camPos = pCamera->GetPosition();

	// SpotLight 후보 수집
	for (int i = 1; i < vectorLightControl.size(); ++i)
	{
		LightControl& lc = vectorLightControl[i];
		Light& light = lc.GetLight();

		//해당 조명이 사용중이 아니다
		if (light.bIsUse == 0)
			continue;

		// 뷰 프러스텀 테스트
		if (!pCamera->m_BoundingFrustumCamera.Intersects(
			lc.GetBoundingSphere()))
		{
			continue;
		}


		// 카메라와 거리 계산
		XMFLOAT3 diff = XMFLOAT3(
			light.position.x - camPos.x,
			light.position.y - camPos.y,
			light.position.z - camPos.z
		);

		float distSq =
			diff.x * diff.x +
			diff.y * diff.y +
			diff.z * diff.z;

		candidates.push_back({ &lc, distSq });
	}

	// 거리 기준 정렬 (가까운 순)
	std::sort(candidates.begin(), candidates.end(),
		[](const SpotCandidate& a, const SpotCandidate& b)
		{
			return a.distance < b.distance;
		});

	// pLights에 복사
	const int MAX_SPOT = MAX_LIGHTS - 1;
	int count = min((int)candidates.size(), MAX_SPOT);

	for (int i = 0; i < count; ++i)
	{
		Light light = candidates[i].ctrl->GetLight();
		BoundingSphere bs = candidates[i].ctrl->GetBoundingSphere();


		// Shadow / 기존 파이프라인용
		pLights[i + 1] = light;
		pLights[i + 1].bIsShadowSet = (i < 4) ? 1 : 0;

		// Cluster / PS용 LightingData

		//LightingData& ld = pLightingdata[i];

		pLightingdata[i].positionW = light.position;
		pLightingdata[i].radius = bs.Radius;

		pLightingdata[i].directionW = light.direction;

		// radiance → intensity (단일 스칼라)
		pLightingdata[i].intensity =
			(light.radiance.x +
				light.radiance.y +
				light.radiance.z) / 3.0f;

		pLightingdata[i].Type = light.type;
	}

}

void CMainScene::PrePareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	int nDirectionResource = m_pShadowDepthTexture->GetTextures();

	m_ShadowIndex = 0;

	//조명 정렬
	Lightsort(m_pLights, m_pLightData,m_pCamera , m_vectorLightControl);

	if (m_pCamera && m_bIsShadow)
	{
		//카메라 안에 조명
		m_pCamera->RegenerateLightViewProjMatrix(m_pLights);
	}

	m_pcbCluster->clusterDim.x = m_ClusterDim.x;
	m_pcbCluster->clusterDim.y = m_ClusterDim.y;
	m_pcbCluster->clusterDim.z = m_ClusterDim.z;
	m_pcbCluster->totalClusters = m_pcbCluster->clusterDim.x *m_pcbCluster->clusterDim.y *m_pcbCluster->clusterDim.z;
	m_pcbCluster->nearZ = m_pCamera->m_fnearZ;
	m_pcbCluster->farZ = m_pCamera->m_fFarZ;
	m_pcbCluster->screenSize.x = FRAME_BUFFER_WIDTH;
	m_pcbCluster->screenSize.y = FRAME_BUFFER_HEIGHT;
	m_pcbCluster->invScreenSize.x = 1.0f / FRAME_BUFFER_WIDTH;
	m_pcbCluster->invScreenSize.y = 1.0f / FRAME_BUFFER_HEIGHT;

	//조명 Update
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(Light) * m_nLights);
	::memcpy(m_pcbLightData, m_pLightData, sizeof(LightingData) * (m_nLights-1)); //클러스터링 렌더링에 사용할 데이터 가공해서 저장중.

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_cpd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(13, d3dGpuVirtualAddress);

	d3dGpuVirtualAddress = m_cpcbCluster->GetGPUVirtualAddress(); //클러스터링 정보 PS에 연결 하는 부분
	pd3dCommandList->SetGraphicsRootConstantBufferView(25, d3dGpuVirtualAddress);


	if (m_bIsShadow)
	{
		for (int i = 0; i < SHADOWMAPS; i++)
		{
			//오류 예방 차원
			if (m_pLights[i].bIsShadowSet == 0)
			{
				continue;
			}
			D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = GetDsvCPUDescriptorHandle(i);
			::SynchronizeResourceTransition(pd3dCommandList, GetShadowMapResource(i), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH,
				1.0f, 0, 0, NULL);
			pd3dCommandList->OMSetRenderTargets(0, NULL, false, &d3dDsvCPUDescriptorHandle);

			pd3dCommandList->SetGraphicsRoot32BitConstants(21, 1, &m_ShadowIndex, 0); //ShadowIndex
			pd3dCommandList->RSSetScissorRects(1, &m_pCamera->m_d3dShadowScissorRect[i]);
			pd3dCommandList->RSSetViewports(1, &m_pCamera->m_d3dShadowMapViewport[i]);
			
			
			OnlyShadowRender(pd3dCommandList, m_pCamera);
			
			m_ShadowIndex += 1;
			
			::SynchronizeResourceTransition(pd3dCommandList, GetShadowMapResource(i), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		}
	}
}

//그림자를 위한 처리를 할 것이다
void CMainScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{
	CScene::OnPreRender(pd3dCommandList, nRenderTargets, pd3dRtvCPUHandles , pd3dDsvCPUHandle, pCamera);
	int nResource = m_pRenderTexture->GetTextures();
	int nResource2 = m_pComputeRenderTexture->GetTextures() - 2;
	D3D12_CPU_DESCRIPTOR_HANDLE* pd3dAllRtvCPUHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets + nResource + nResource2];
	//Render에 있었는데 여기로 DescriptorHeaps 연결을 옮김,
	pd3dCommandList->SetDescriptorHeaps(1, m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap.GetAddressOf());
	//RenderTarget을 Clear해주고 있음
	for (int i = 0; i < nRenderTargets; i++)
	{
		pd3dAllRtvCPUHandles[i] = pd3dRtvCPUHandles[i];
		pd3dCommandList->ClearRenderTargetView(pd3dRtvCPUHandles[i], Colors::Black, 0, NULL);
	}

	//후처리에 사용할 추가적인 RTV에 SRV 갯수만큼 반복하며 연결중.
	for (int i = 0; i < nResource; i++)
	{
		//생성됬을땐 STATE_COMMON 상태이고 RenderTarget으로 사용하기 위해 상태 전이

		::SynchronizeResourceTransition(pd3dCommandList, GetRenderTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, Colors::Black, 0, NULL);
		pd3dAllRtvCPUHandles[nRenderTargets + i] = d3dRtvCPUDescriptorHandle;
	}

	for (int i = 0; i < nResource2; i++)
	{
		//생성됬을땐 STATE_COMMON 상태이고 RenderTarget으로 사용하기 위해 상태 전이

		::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(i), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetG_BufferRtvCPUDescriptorHandle(i);
		pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, Colors::Black, 0, NULL);
		pd3dAllRtvCPUHandles[nRenderTargets+ nResource + i] = d3dRtvCPUDescriptorHandle;
	}

	//그림자를 그려라
	PrePareShadowMap(pd3dCommandList, pCamera);

	//클러스터링 작업
	//컴퓨트 파이프라인을 동작 시킨다
	::SynchronizeResourceTransition(pd3dCommandList, m_cpsbClusterLightList.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	D3D12_GPU_VIRTUAL_ADDRESS d3dLightDataGPUAddress = m_cpcbLightData->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS d3dClusterDataGPUAddress = m_cpcbCluster->GetGPUVirtualAddress();

	if (m_pCamera)
	{
		m_pCamera->UpdateShaderVariables(pd3dCommandList);
		m_pCamera->UpdateShaderVariablesCompute(pd3dCommandList);
	}

	//Cluster 생성
	m_pComputeShader->OnPreparePSO(pd3dCommandList, 6);
	pd3dCommandList->SetComputeRootConstantBufferView(1, d3dClusterDataGPUAddress);
	pd3dCommandList->SetComputeRootConstantBufferView(7, d3dLightDataGPUAddress);
	pd3dCommandList->SetComputeRootDescriptorTable(5, m_hClusterLightListUav);
	pd3dCommandList->Dispatch(m_ClusterDim.x, m_ClusterDim.y, m_ClusterDim.z);

	::SynchronizeUAVBarrier(pd3dCommandList, m_cpsbClusterLightList.Get());
	::SynchronizeResourceTransition(pd3dCommandList, m_cpsbClusterLightList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	
	//가공이 끝난 클러스터드 인덱스를 그래픽스 PSO에 전달
  // 클러스터링에 저장된 인덱스 정보
	pd3dCommandList->SetGraphicsRootDescriptorTable(26, m_hClusterLightListSrv);

	//////

	D3D12_CPU_DESCRIPTOR_HANDLE* hdrAndNormalRtvs = new D3D12_CPU_DESCRIPTOR_HANDLE[nResource + nResource2];
	hdrAndNormalRtvs[0] = pd3dAllRtvCPUHandles[1]; // HDR
	hdrAndNormalRtvs[1] = pd3dAllRtvCPUHandles[2]; // Normal
	hdrAndNormalRtvs[2] = pd3dAllRtvCPUHandles[3]; // IBL : Ambient
	//후처리에 사용할 HDR RenderTargets으로 RenderTargets를 지정
	pd3dCommandList->OMSetRenderTargets(3, hdrAndNormalRtvs, TRUE, pd3dDsvCPUHandle);


	if (pd3dAllRtvCPUHandles)
	{
		delete[] pd3dAllRtvCPUHandles;
	}
	if (hdrAndNormalRtvs)
	{
		delete[] hdrAndNormalRtvs;
	}
}

void CMainScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	bIsFirstAnimate = true;
	CScene::Render(pd3dCommandList, m_pCamera);
	
	pd3dCommandList->SetGraphicsRootDescriptorTable(20, m_pShadowDepthTexture->GetGpuDescriptorHandle(0));
	//SkyBox는 Depth Draw에 참여 안함
	if (m_pSkyBox && m_nWeather == 0)
	{
		m_pSkyBox->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}
	else if (m_pSkyBoxCloud)
	{
		m_pSkyBoxCloud->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}

	//
	if (m_pPlan)
	{
		m_pPlan->Render(pd3dCommandList, m_pCamera, true, false, m_bIsShowWire, false);
	}
	//
	//
	if (m_pTerrain)
	{
		m_pTerrain->Render(pd3dCommandList, m_pCamera, true, false, m_bIsShowWire, false);
	}
	//
	//
	if (m_pBuildingObject->GetActivie())
	{
	
		//m_pBuildingObject->PrintMatrixWorld();
		//건물 설치 관련해서 ==> 이게 최상위 부모 노드에서 갖고있는 값을 반영해야해서
		//여기서 처리한다. but 좀 더 이쁘게 정리할려면 이 m_pBuildingObject를 별개의
		//오브젝트로 처리하면 된다.
		//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 1, &m_pBuildingObject->m_nIsBuildingObject, 36);
		m_pBuildingObject->UpdateTransform(NULL);
		m_pBuildingObject->Render(pd3dCommandList, m_pCamera, true, false , false ,false);
	}
	//

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->GetActivie())
		{
			//뷰 프러스텀 컬링
			if (!m_pCamera->m_BoundingFrustumCamera.Intersects(
				m_ppHierarchicalGameObjects[i]->m_obbBox))
			{
				continue;
			}
			//예제에선 여기섯 Animate와 UpdateTransform까지 다 했지만 
			//Render에는 최대한 Render만 넣을려고 한다. 횟수만 따졌을땐
			//최대한 하나로 합치는게 좋겠지만 말이다. 최적화 입장이 아닌 접근에 용이하게 작성한 케이스
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) //null이면 
			{
				m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			}
			//Update Transform을 이때 해야 제대로 계층구조가 반영된다.
			//m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera, true, false, m_bIsShowWire , false);

			if (m_bIsShowNormals)
			{
				m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera, true, m_bIsShowNormals, m_bIsShowWire, false);
			}
		}
	}

	if (m_bIsShowNormals) //Normals가 true이면 Normal을 그려준다
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++)
		{
			if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->GetActivie())
			{
			
				if (m_bIsShowNormals)
				{
					m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera, true, m_bIsShowNormals, m_bIsShowWire, false);
				}
			}
		}
		if (m_pPlan)
		{
			m_pPlan->Render(pd3dCommandList, m_pCamera, true, m_bIsShowNormals, m_bIsShowWire, false);
		}
		if (m_pTerrain)
		{
			m_pTerrain->Render(pd3dCommandList, m_pCamera, true, false, m_bIsShowWire, true);
		}
	}

	if (m_bIsShowBoundingBox) //OBB 박스 그려라 
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++)
		{
			//m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (m_ppHierarchicalGameObjects[i] && m_ppHierarchicalGameObjects[i]->GetActivie())
			{
				//m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, pCamera, true, m_bIsShowNormals, m_bIsShowWire);
				m_ppHierarchicalGameObjects[i]->DebugRender(pd3dCommandList, m_pCamera);
			}
		}
		if (m_pPlayer)
		{
			m_pPlayer->DebugRender(pd3dCommandList, m_pCamera);
		}
	}

	if (m_pPlayer)
	{
		//같은 모델을 공유할 경우엔 사용 이렇게 Animate해줘야 한다 ==> 이거 틀린거임 뭔가 CommandList랑 메모리상에 배열 문제인거같음
		m_pPlayer->Animate(m_fElapsedTime);
		m_pPlayer->Render(pd3dCommandList, m_pCamera);
	}


}

void CMainScene::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles
	, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{
	//컴퓨트 파이프라인을 동작 시킨다
	//m_pComputeShader->Render(pd3dCommandList, m_pCamera);
	//m_pComputeShader->OnPreparePSO(pd3dCommandList, 0); // PostPorcessing : 0, BlurX : 1, BlurY :2, SSAO : 3
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(1), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, GetRenderTextureResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, m_pDefaultDepthStancilResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	pd3dCommandList->SetComputeRootDescriptorTable(2, GetDsSrvGPUDescriptorHandle());//깊이맵
	pd3dCommandList->SetComputeRootDescriptorTable(3, m_pComputeRenderTexture->GetGpuDescriptorHandle(0)); //노멀맵
	pd3dCommandList->SetComputeRootDescriptorTable(4, m_pComputeRenderTexture->GetGpuDescriptorHandle(1)); //IBL

	//SSAO 생성
	m_pComputeShader->OnPreparePSO(pd3dCommandList, 3);
	pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(2)); //SSAO결과를 저장한다
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(2));
	//::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	
	//Blur
	for (int i = 0; i < 5; i++)
	{

		::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_pComputeShader->OnPreparePSO(pd3dCommandList, 1);
		pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(2));
		pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(3));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
		::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(3));

		::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


		m_pComputeShader->OnPreparePSO(pd3dCommandList, 2);
		pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(3));
		pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
		::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(2));

	}

	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(0), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//SSAO 적용중
	m_pComputeShader->OnPreparePSO(pd3dCommandList, 0);
	pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(2));
	pd3dCommandList->SetComputeRootDescriptorTable(5, m_pRenderTexture->GetUavGpuDescriptorHandle(0)); //SSAO 적용한 HDRI
	pd3dCommandList->SetComputeRootDescriptorTable(6, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(0)); //Bloom넘길거
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);

	::SynchronizeUAVBarrier(pd3dCommandList, GetRenderTextureResource(0));
	::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(1));


	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//Bloom 
	m_pComputeShader->OnPreparePSO(pd3dCommandList, 4);
	pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(0));
	pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(2));

	if (m_bIsUseBLOOM)
	{
		//Blur
		for (int i = 0; i < 10; i++)
		{

			::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_pComputeShader->OnPreparePSO(pd3dCommandList, 1);
			pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(2));
			pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(3));
			pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
				UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
			::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(3));
			::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(2));
			::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			m_pComputeShader->OnPreparePSO(pd3dCommandList, 2);
			pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(3));
			pd3dCommandList->SetComputeRootDescriptorTable(5, m_pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
			pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
				UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
			::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(2));
			::SynchronizeUAVBarrier(pd3dCommandList, GetG_BufferRenderTextureResource(3));
		}
	}
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(3), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	//Combine
	m_pComputeShader->OnPreparePSO(pd3dCommandList, 5);
	pd3dCommandList->SetComputeRootDescriptorTable(2, m_pComputeRenderTexture->GetGpuDescriptorHandle(2));
	pd3dCommandList->SetComputeRootDescriptorTable(5, m_pRenderTexture->GetUavGpuDescriptorHandle(0));
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, GetRenderTextureResource(0));
	// UAV -> SRV
	::SynchronizeResourceTransition(pd3dCommandList, GetRenderTextureResource(0), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);


	//Compute SRV-> SRV
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(0), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(1), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, GetG_BufferRenderTextureResource(2), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);

	::SynchronizeResourceTransition(pd3dCommandList, m_pDefaultDepthStancilResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = GetRtvCPUDescriptorHandle(0);

	//SSAO, Bloom과 같은 후처리에 영향을 받지 않도록 particle을 끝난 이후 따로 Render 해준다 ==> 우선 Ambient에만 영향을 받을거고 
	//PBR Particle이 아닌 일반 Particle이므로 SSAO와 Bloom 이상한 영향을 미친다.
	pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, pd3dDsvCPUHandle);

	if (m_pRainParticle && m_pRainParticle->GetActivie())
	{
		m_pRainParticle->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}
	if (m_pSnowParticle && m_pSnowParticle->GetActivie())
	{
		m_pSnowParticle->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}

	if (m_pDustParticle && m_pDustParticle->GetActivie())
	{
		m_pDustParticle->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}

	if (m_pPunchParticleObject && m_pPunchParticleObject->GetActivie())
	{
		m_pPunchParticleObject->Render(pd3dCommandList, m_pCamera, true, false, false, false);
	}


	::SynchronizeResourceTransition(pd3dCommandList, m_pDefaultDepthStancilResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, GetRenderTextureResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	//최종 기본으로 출력할 LDR로 전환 이후 PostProcessing을 지원하는 오브젝트로 Render
	pd3dCommandList->OMSetRenderTargets(1, pd3dRtvCPUHandles, TRUE, NULL);
	//pd3dCommandList->OMSetRenderTargets(1,,,);

	//GetDsSrvGPUDescriptorHandle() 현재 가장 기본 깊이맵임.

	//HDR 이미지를 최종적 Set해주고 그려준다
	if (pCamera)
	{
		pCamera->UpdateShaderVariables(pd3dCommandList); //카메라의 상수 버퍼나 여러 GPU나 Shader에 넘겨야할 명령어등을 Update해주는 부분
	}
	pd3dCommandList->SetGraphicsRootDescriptorTable(19, m_pRenderTexture->GetGpuDescriptorHandle(0));
	pd3dCommandList->SetGraphicsRootDescriptorTable(4, GetDsSrvGPUDescriptorHandle()); //깊이맵
	m_pPostProcessingObject->Render(pd3dCommandList, pCamera);


	::SynchronizeResourceTransition(pd3dCommandList, m_pDefaultDepthStancilResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	



}
void CMainScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_cpd3dcbImGui_Info = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpd3dcbImGui_Info->Map(0, NULL, (void**)&m_pcbMaterialFlages);

	ncbElementBytes = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255); //256의 배수
	m_cpd3dcbImGui_ComputeInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpd3dcbImGui_ComputeInfo->Map(0, NULL, (void**)&m_pcbComputeFlages);


	ncbElementBytes = ((sizeof(Light) + 255) & ~255); //256의 배수
	m_cpd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	ncbElementBytes = ((sizeof(Light) + 255) & ~255); //256의 배수
	m_cpd3dcbSSAOKernel = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpd3dcbSSAOKernel->Map(0, NULL, (void**)&m_pcbSSAOKernel);

	CreateSSAOKernel();

	//나중에 날씨 변경되는거에 따라서 여러개를 한번에 구워두거나 아니면 날씨가 변할때 굽는 방식으로 변경해야함.
	m_pIBLBRDFTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pIBLBRDFTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/MySkysBrdf.dds", RESOURCE_TEXTURE2D, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLBRDFTexture, 0, 16);

	m_pIBLSpecluarTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	m_pIBLSpecluarTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/MySkysSpecularHDR.dds", RESOURCE_TEXTURE_CUBE, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLSpecluarTexture, 0, 17);

	m_pIBLIrradianceTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	m_pIBLIrradianceTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/MySkysDiffuseHDR.dds", RESOURCE_TEXTURE_CUBE, 0);
	
	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLIrradianceTexture, 0, 18);

	//눈오는 날씨와 비오는 날씨
	m_pIBLBRDFTextureCloud = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pIBLBRDFTextureCloud->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/CloudSkyBrdf.dds", RESOURCE_TEXTURE2D, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLBRDFTextureCloud, 0, 16);

	m_pIBLSpecluarTextureCloud = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	m_pIBLSpecluarTextureCloud->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/CloudSkySpecularHDR.dds", RESOURCE_TEXTURE_CUBE, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLSpecluarTextureCloud, 0, 17);

	m_pIBLIrradianceTextureCloud = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	m_pIBLIrradianceTextureCloud->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/CloudSkyDiffuseHDR.dds", RESOURCE_TEXTURE_CUBE, 0);

	CScene::CreateShaderResourceViews(pd3dDevice, m_pIBLIrradianceTextureCloud, 0, 18);
}

void CMainScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CScene::UpdateShaderVariables(pd3dCommandList);

	Toggle maptoggles[] = {
	{ &m_bIsUseAlbedo,     MATERIAL_ALBEDO_MAP_USE },
	{ &m_bIsUseNormal,     MATERIAL_NORMAL_MAP_USE },
	{ &m_bIsUseRoughness,  MATERIAL_ROUGHNESS_MAP_USE },
	{ &m_bIsUseMetalness,  MATERIAL_METALNESS_MAP_USE },
	{ &m_bIsUseAO,         MATERIAL_AO_MAP_USE },
	{ &m_bIsUseSpecularF0, MATERIAL_SPECULARF0_MAP_USE },
	{ &m_bIsUseEmission,   MATERIAL_EMISSION_MAP_USE },
	};

	Toggle computeFlagetoggles[] =
	{
		{&m_bIsUseSSAO, MATERIAL_SSAO_USE } ,
		{&m_bIsUseBLOOM, MATERIAL_BLOOM_USE}
	};

	//재질에 환경광이랑 DIrection광 적용 여부
	Toggle lighttoggles[] =
	{
		{&m_bIsUsedirect, DIRECT_LIGHT_USE },
		{&m_bIsUseambient, AMBIENT_LIGHT_USE},
		{&m_bIsUseDirectionLight, DIRECTION_LIGHT_USE},
	};

	UnsetSeasonType(SEASON_SPRING);
	UnsetSeasonType(SEASON_SUMMER);
	UnsetSeasonType(SEASON_FALL);
	UnsetSeasonType(SEASON_WINTER);

	// 선택된 시즌 하나만 설정
	switch (m_nSeasonIndex)
	{
	case 0:
	{
		SetSeasonType(SEASON_SPRING); break;
	}
	case 1:
	{
		SetSeasonType(SEASON_SUMMER); break;
	}
	case 2:
	{
		SetSeasonType(SEASON_FALL);   break;
	}
	case 3:
	{
		SetSeasonType(SEASON_WINTER); break;
	}
	default:
	{
		SetSeasonType(SEASON_SPRING); break;
	}
	}

	UnsetSetWheatherType(WEATHER_CLEAR);
	UnsetSetWheatherType(WEATHER_RAIN);
	UnsetSetWheatherType(WEATHER_SNOW);
	UnsetSetWheatherType(WEATHER_DUST);
	UnsetSetWheatherType(WEATHER_FOG);

	switch (m_nWeather)
	{
	case 0:
	{
		SetWheatherType(WEATHER_CLEAR); break;
	}
	case 1:
	{
		SetWheatherType(WEATHER_RAIN); break;
	}
	case 2:
	{
		SetWheatherType(WEATHER_SNOW);   break;
	}
	case 3:
	{
		SetWheatherType(WEATHER_DUST); break;
	}
	case 4:
	{
		SetWheatherType(WEATHER_FOG); break;
	}
	default:
	{
		SetWheatherType(SEASON_SPRING); break;
	}

	}


	for (auto& t : computeFlagetoggles)
	{
		if (*t.state)
		{
			SetComputeFlagType(t.flag);
		}
		else
		{
			UnsetComputeFlagType(t.flag);
		}
	}

	for (auto& t : maptoggles)
	{
		if (*t.state)
		{
			SetMaterialType(t.flag);
		}
		else
		{
			UnsetMaterialType(t.flag);
		}
	}

	for (auto& t : lighttoggles)
	{
		if (*t.state)
		{
			SetLightType(t.flag);
		}
		else
		{

			UnsetLightType(t.flag);
		}
	}

	m_pcbMaterialFlages->m_fpExposure = m_fpExposure;
	m_pcbMaterialFlages->m_fpGamma = m_fpGamma;
	m_pcbMaterialFlages->m_fpRoughness = m_fpRoughnessValue;
	m_pcbMaterialFlages->m_fpMetalness = m_fpMetalness;

	m_pcbMaterialFlages->m_nShadowOnOFF = m_bIsShadow ? 1 : 0;

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_cpd3dcbImGui_Info->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(14, d3dGpuVirtualAddress);

	//TODO : 이거 순서 땜에 오류 날 수 있으니까 오류 나거나 하면 ComputeShader Render하는 부분으로 옮겨야함
	XMStoreFloat4x4(&m_pcbComputeFlages->m_xmf4x4invPorj,
		XMMatrixTranspose(XMLoadFloat4x4(&m_pCamera->GetInvProjectinMatrix())));
	XMStoreFloat4x4(&m_pcbComputeFlages->m_xmf4x4Proj,
		XMMatrixTranspose(XMLoadFloat4x4(&m_pCamera->GetProjectinMatrix())));
	m_pcbComputeFlages->m_xmf2Radius = m_fRadiusX;
	m_pcbComputeFlages->m_fBias = m_fBias;
	m_pcbComputeFlages->BloomThreshold = m_fBloomThreshold;
	m_pcbComputeFlages->BloomSoftKnee = m_fBloomSoftKnee;
	m_pcbComputeFlages->BloomIntensity = m_fBloomIntensity;
	d3dGpuVirtualAddress = m_cpd3dcbImGui_ComputeInfo->GetGPUVirtualAddress();
	pd3dCommandList->SetComputeRootConstantBufferView(0, d3dGpuVirtualAddress);

	d3dGpuVirtualAddress = m_cpd3dcbSSAOKernel->GetGPUVirtualAddress();
	pd3dCommandList->SetComputeRootConstantBufferView(1, d3dGpuVirtualAddress);

	switch (m_nWeather)
	{
	case 0: //맑은날
	{
		m_pRainParticle->Activate(false);
		m_pSnowParticle->Activate(false);
		m_pDustParticle->Activate(false);
		break;
	}
	case 1: //비 오는 날
	{
		m_pRainParticle->Activate(true);
		m_pSnowParticle->Activate(false);
		m_pDustParticle->Activate(false);
		break;
	}
	case 2: //눈 오는 날
	{
		m_pRainParticle->Activate(false);
		m_pSnowParticle->Activate(true);
		m_pDustParticle->Activate(false);
		break;
	}
	case 3: //황사
	{
		m_pRainParticle->Activate(false);
		m_pSnowParticle->Activate(false);
		m_pDustParticle->Activate(true);
		break;
	}
	case  4: //안개
	{
		m_pRainParticle->Activate(false);
		m_pSnowParticle->Activate(false);
		m_pDustParticle->Activate(false);
		break;
	}
	default:
		break;
	}

}

void CMainScene::OnGUI(ImGuiIO* pGuiIO)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);
	ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

	// 여기서 디버그/툴 UI 작성 //
	ImGui::Text("FrameTime: %.3f ms", 1000.0f / pGuiIO->Framerate);

	// Normal 표시 토글 ==> PBR 설정과 조명 설정이 어느정도 되고 
	ImGui::Checkbox("Show Normal Vectors", &m_bIsShowNormals);
	ImGui::Checkbox("Show Wire Mesh", &m_bIsShowWire);
	ImGui::Checkbox("Show Bounding Box", &m_bIsShowBoundingBox);
	ImGui::Checkbox("Albedo Map", &m_bIsUseAlbedo);
	ImGui::Checkbox("Normal Map", &m_bIsUseNormal);
	ImGui::Checkbox("Roughness Map", &m_bIsUseRoughness);
	ImGui::Checkbox("Metalness Map", &m_bIsUseMetalness);
	ImGui::Checkbox("AO Map", &m_bIsUseAO);
	ImGui::Checkbox("Specular F0 Map", &m_bIsUseSpecularF0);
	ImGui::Checkbox("Emission Map", &m_bIsUseEmission);
	ImGui::Checkbox("Direct Light Use", &m_bIsUsedirect);
	ImGui::Checkbox("Ambient Light Use", &m_bIsUseambient);
	ImGui::Checkbox("Direction Light Use", &m_bIsUseDirectionLight);
	ImGui::SliderFloat("Gamma", &m_fpGamma, 0.0f, 10.0f);
	ImGui::SliderFloat("Roughness", &m_fpRoughnessValue, 0.0f, 1.0f);
	ImGui::SliderFloat("Metalness", &m_fpMetalness, 0.0f, 1.0f);
	ImGui::Checkbox("Show Shadow", &m_bIsShadow);
	ImGui::Checkbox("SSAO USE", &m_bIsUseSSAO);
	ImGui::Text("SSAO Radius");
	ImGui::SliderFloat("Radius_X", &m_fRadiusX, 0.05f, 1.5f);
	ImGui::SliderFloat("SSAO Bias", &m_fBias, 0.001f, 0.1f);
	ImGui::Checkbox("BLOOM USE", &m_bIsUseBLOOM);
	ImGui::SliderFloat("Threshold", &m_fBloomThreshold, 0.5f, 2.0f);
	ImGui::SliderFloat("SoftKnee", &m_fBloomSoftKnee, 0.0f, 1.0f);
	ImGui::SliderFloat("BloomIntensity", &m_fBloomIntensity, 0.0f, 2.0f);
	ImGui::Text("Weather");
	bool worldStateChanged = false;
	worldStateChanged |= ImGui::RadioButton("Sunny", &m_nWeather, 0);
	worldStateChanged |= ImGui::RadioButton("Rain", &m_nWeather, 1);
	worldStateChanged |= ImGui::RadioButton("Snow", &m_nWeather, 2);
	worldStateChanged |= ImGui::RadioButton("Dust", &m_nWeather, 3);
	worldStateChanged |= ImGui::RadioButton("Fog", &m_nWeather, 4);

	ImGui::Text("Season");
	worldStateChanged |= ImGui::RadioButton("Spring", &m_nSeasonIndex, 0);
	worldStateChanged |= ImGui::RadioButton("Summer", &m_nSeasonIndex, 1);
	worldStateChanged |= ImGui::RadioButton("Fall", &m_nSeasonIndex, 2);
	worldStateChanged |= ImGui::RadioButton("Winter", &m_nSeasonIndex, 3);

	if (worldStateChanged && m_upClientNetwork)
	{
		Protocol::C_DEBUG_WORLD_STATE_CHANGE pkt;
		pkt.set_season(static_cast<uint32>(m_nSeasonIndex));
		pkt.set_weather(static_cast<uint32>(m_nWeather));
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		m_upClientNetwork->SendToServer(sendBuffer);
	}



	static float teleportPos[3] = { 0.0f, 0.0f, 0.0f };

	// 숫자 3개 입력 (X, Y, Z)
	ImGui::InputFloat3("Position (X Y Z)", teleportPos);

	// 버튼
	if (ImGui::Button("Teleport"))
	{
		// 버튼 눌렀을 때만 실제 반영
		m_pPlayer->SetPosition(XMFLOAT3(teleportPos[0], teleportPos[1], teleportPos[2])
		);
	}

	ImGui::End();
	if (GUIOnOFF)
	{

		//게임 GUI
		ImGui::SetNextWindowPos(ImVec2(10, 510), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Once);
		ImGui::Begin("Building Option", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Building Object Select");
		ImGui::RadioButton("Wall", &m_nBuildingObjectIndex, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Floor", &m_nBuildingObjectIndex, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Ceiling", &m_nBuildingObjectIndex, 2);
		ImGui::RadioButton("CoonSeed", &m_nBuildingObjectIndex, 3);
		ImGui::SameLine();
		ImGui::RadioButton("PotatoSeed", &m_nBuildingObjectIndex, 4);
		ImGui::SameLine();
		ImGui::RadioButton("PumpkinSeed", &m_nBuildingObjectIndex, 5);
		ImGui::SameLine();
		ImGui::RadioButton("TomatoSeed", &m_nBuildingObjectIndex, 6);
		ImGui::RadioButton("LogPlanter", &m_nBuildingObjectIndex, 7);

		ImGui::End();
	}

	const float inventoryWidth = 260.0f;
	const float inventoryHeight = 220.0f;

	ImGui::SetNextWindowPos(
		ImVec2(
			FRAME_BUFFER_WIDTH - inventoryWidth - 10.0f,
			10.0f
		),
		ImGuiCond_Always
	);

	ImGui::SetNextWindowSize(
		ImVec2(inventoryWidth, inventoryHeight),
		ImGuiCond_Always
	);

	ImGui::Begin("Inventory", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse
	);

	Player_Inventory& inv = m_pPlayer->m_pPlayerInventory;

	ImGui::Text("=== Inventory ===");
	ImGui::Separator();

	ImGui::Text("Resources");
	ImGui::BulletText("Rock        : %d", inv.Rock);
	ImGui::BulletText("Wood        : %d", inv.wood);

	ImGui::Spacing();
	ImGui::Text("Seeds");
	ImGui::BulletText("Corn Seed   : %d", inv.coonSeed);
	ImGui::BulletText("Potato Seed : %d", inv.potatoSeed);
	ImGui::BulletText("Pumpkin Seed: %d", inv.pumpkinSeed);
	ImGui::BulletText("Tomato Seed : %d", inv.tomatoSeed);

	ImGui::BulletText("X : %.2f", m_fPlayerPositionX);
	ImGui::BulletText("Z : %.2f", m_fPlayerPositionZ);
	ImGui::End();

	const float scoreWindowWidth = 100.0f;
	const float scoreWindowHeight = 20.0f;

	ImGui::SetNextWindowPos(
		ImVec2(
			(FRAME_BUFFER_WIDTH * 0.5f) - (scoreWindowWidth * 0.5f),
			10.0f
		),
		ImGuiCond_Always
	);

	ImGui::SetNextWindowSize(
		ImVec2(scoreWindowWidth, scoreWindowHeight),
		ImGuiCond_Always
	);

	ImGui::Begin(
		"Score",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoInputs
	);

	//ImGui::SetWindowFontScale(1.8f);
	// 가운데 정렬
	ImGui::SetCursorPosX(
		(scoreWindowWidth - ImGui::CalcTextSize("Score : 000000").x) * 0.5f
	);

	ImGui::Text("Score : %.0f", m_fScore);

	ImGui::End();
}



void CMainScene::OnEnter()
{
	
}

//
void CMainScene::SelectObject(CLoadedModelInfo* pModel)
{
	if (!pModel) //아무것도 없는 null 넘겼다면
	{
		return;
	}
	if (m_pBuildingObject->m_pChild)
	{
		m_pBuildingObject->m_pChild = NULL;
	}
	//아직 선택된게 없는 경우 Render를 활성화 한다.
		for (int i = 0; i < m_pBuildingObject->m_nOBBData; i++)
		{
			//같은 타입이라면
			if (m_pBuildingObject->m_pOBBData[i].m_objectType == pModel->m_ObjectType)
			{
				//현재 이 OBB Data로 갱신을 해준다.
				m_pBuildingObject->ResetOBB(m_pBuildingObject->m_pOBBData[i]);
				break;
			}
		}
		//m_pBuildingObject->m_pChild = NULL;
		m_pBuildingObject->SetChild(pModel->m_pModelRootObject, false);
		m_pBuildingObject->m_nIsBuildingObject = 1; //아직 다른 오브젝트와의 충돌 기능이
		//안들어있어서 여기서 바로 1로 바꿔준다. 못지으면 2
		m_pBuildingObject->Activate(true);

		if (m_pPlayer)
		{
			m_pCamera = m_pPlayer->ChangeCamera(FIRST_PERSON_CAMERA,
				m_pGameTimer->GetTimeElapsed());
		}
	
}

//B버튼을 눌럿을때 활성화 지
void CMainScene::SelectObjects()
{
	if (m_pBuildingObject->m_nIsBuildingObject != 0) //이미 활성화 된 상태여서 비활성 상태로 만들고
	{
		m_pBuildingObject->m_pChild = NULL;
		m_pBuildingObject->m_nIsBuildingObject = 0; //여긴 0으로 바꾸는게 맞다
		m_pBuildingObject->Activate(false);

		if (m_pPlayer)
		{
			m_pCamera = m_pPlayer->ChangeCamera(THIRD_PERSON_CAMERA,
				m_pGameTimer->GetTimeElapsed());
		}
		return;
	}
	CLoadedModelInfo* model =NULL;
	switch (m_nBuildingObjectIndex)
	{
	case 0: //벽
		{
			model = m_pWallModel;
			break;
		}
	case 1: //바닥
		{
			model = m_pfloorModel;
			break;
		}
	case 2: // 천장
		{
			model = m_pceilingModel;
			break;
		}
	case 3: //옥수수
		{
			model = m_pCornSeedModel;
			break;
		}
	case 4: //감자
		{
			model = m_pPotatoSeedModel;
			break;
		}
	case 5: //호박
		{
			model = m_pPumpkinSeedModel;
			break;
		}
	case 6: //토마토
		{
			model = m_pTomatoSeedModel;
			break;
		}
	case 7: //밭
		{
			model = m_pLogPlanterModel;
			break;
		}
	default:
		{
			break;
		}
	}
	SelectObject(model);
	m_nPreviousBuildingObjectIndex = m_nBuildingObjectIndex;

}

void CMainScene::BuildingObjectCheck()
{
	if (!m_pBuildingObject->GetActivie()) // false면 처리하지 않는다
	{
		m_nPreviousBuildingObjectIndex = -1; //
		return;
	}

	if (m_nBuildingObjectIndex != m_nPreviousBuildingObjectIndex)
	{
		CLoadedModelInfo* model = NULL;
		switch (m_nBuildingObjectIndex)
		{
		case 0: //벽
		{
			model = m_pWallModel;
			break;
		}
		case 1: //바닥
		{
			model = m_pfloorModel;
			break;
		}
		case 2: // 천장
		{
			model = m_pceilingModel;
			break;
		}
		case 3: //옥수수
		{
			model = m_pCornSeedModel;
			break;
		}
		case 4: //감자
		{
			model = m_pPotatoSeedModel;
			break;
		}
		case 5: //호박
		{
			model = m_pPumpkinSeedModel;
			break;
		}
		case 6: //토마토
		{
			model = m_pTomatoSeedModel;
			break;
		}
		case 7: //밭
		{
			model = m_pLogPlanterModel;
			break;
		}
		default:
		{
			break;
		}
		}
		SelectObject(model);
		m_nPreviousBuildingObjectIndex = m_nBuildingObjectIndex;
	}
	
}

void CMainScene::BuildingObjectAnimateUpdate(float fTimeElapsed)
{
	BuildingObjectCheck(); //빌딩 모드 관련 Check 사항 ==> 빌딩 모드에 건설 오브젝트를 변경한다.

	//빌딩모드가 켜져있다면
	if (m_pBuildingObject->GetActivie())
	{
		CCamera* pCamera = m_pPlayer->GetCamera();
		XMFLOAT3 camPos = pCamera->GetPosition();
		XMFLOAT3 camDir = pCamera->GetLookVector();

		float distance = 0;
		float maxDistance = 8.0f;

		m_pBuildingObject->m_pHitGameObject = NULL;

		switch (m_pBuildingObject->m_objTypeTag)
		{

		case EObjectTag::Floor:
		{
			/*		m_pBuildingObject->SetPosition(RayToPlanCollision(pCamera->GetPosition(),
						XMFLOAT3(0, 1, 0), pCamera->GetLookVector()));*/
			float nd = camDir.y;
			if (fabs(nd) > 1e-6f) //너무 작은 경우를 배제하기 위해
			{
				XMFLOAT3 hit = RayToTerrainCollision(camPos, camDir, m_pTerrain, distance);

				if (distance < maxDistance)
				{
					m_pBuildingObject->SetPosition(hit);
					m_pBuildingObject->m_nIsBuildingObject = 1; //우선 건설 가능 설정
				}
				else
				{
					m_pBuildingObject->SetPosition(XMFLOAT3(0.0f, 1000, 0.0f));
					m_pBuildingObject->m_nIsBuildingObject = 2;
				}
			}
			for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				//충돌이 활성화되어있는 오브젝트
				if (m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision)
				{
					//OBB와 OBB 충돌 체크
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(m_pBuildingObject->m_obbBox))
					{
						m_pBuildingObject->m_nIsBuildingObject = 2;
						break; //충돌 되는게 하나라도있으면 바로 설치 불가능 상태 표시.
					}
				}
			}
			if (m_pPlayer->m_pPlayerInventory.Rock < 3)
			{
				m_pBuildingObject->m_nIsBuildingObject = 2;
			}
			break;
		}
		case EObjectTag::Wall:
		{

			XMFLOAT3 hit = XMFLOAT3(0.0f, 1000.0f, 0.0f);
			CFloorObject* obj;

			XMVECTOR origin = XMLoadFloat3(&camPos);
			XMVECTOR dir = XMLoadFloat3(&camDir);

			for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				//LogPlanter이면서 충돌이 활성화되어있는 오브젝트
				if (m_ppHierarchicalGameObjects[i]->GetType() == EObjectModelType::Floor && m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision)
				{
					//Ray와 OBB 충돌 체크
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(origin, dir, distance))
					{
						XMFLOAT3 pos = m_ppHierarchicalGameObjects[i]->GetPosition();
						if (distance < maxDistance) //너무 멀리있으면 작동하지 않도록
						{
							XMVECTOR hitPos = origin + dir * distance;
							XMStoreFloat3(&hit, hitPos);
							m_pBuildingObject->m_pHitGameObject = m_ppHierarchicalGameObjects[i];
							m_pBuildingObject->m_nIsBuildingObject = 2;
							obj = (CFloorObject*)m_pBuildingObject->m_pHitGameObject;
							obj->m_nSelectIndex = -1;
							//X+
							if (hit.x > pos.x + 2.0f && hit.x < pos.x + 2.5f &&
								hit.z < pos.z + 1.0f && hit.z > pos.z - 1.0f)
							{
								if (obj->m_npPlaceable[0] == 0)
								{
									m_pBuildingObject->m_nIsBuildingObject = 1;
									obj->m_nSelectIndex = 0;
								}
								m_pBuildingObject->SetRotation(0.0f, 0.0f, 0.0f);
								hit = Vector3::Add(pos, XMFLOAT3(2.375f, 0.5f, 0.0f));
							}
							//X-
							else if (hit.x < pos.x - 2.0f && hit.x > pos.x - 2.5f &&
								hit.z < pos.z + 1.0f && hit.z > pos.z - 1.0f)
							{
								if (obj->m_npPlaceable[2] == 0)
								{
									m_pBuildingObject->m_nIsBuildingObject = 1;
									obj->m_nSelectIndex = 2;
								}
								m_pBuildingObject->SetRotation(0.0f, 0.0f, 0.0f);
								hit = Vector3::Add(pos, XMFLOAT3(-2.375f, 0.5f, 0.0f));
							}
							//Z+
							else if (hit.z > pos.z + 2.0f && hit.z < pos.z + 2.5f &&
								hit.x < pos.x + 1.0f && hit.x > pos.x - 1.0f)
							{
								if (obj->m_npPlaceable[1] == 0)
								{
									m_pBuildingObject->m_nIsBuildingObject = 1;
									obj->m_nSelectIndex = 1;
								}
								m_pBuildingObject->SetRotation(0.0f, 90.0f, 0.0f);
								hit = Vector3::Add(pos, XMFLOAT3(0.0f, 0.5f, 2.375f));
							}
							//Z-
							else if (hit.z < pos.z - 2.0f && hit.z > pos.z - 2.5f &&
								hit.x < pos.x + 1.0f && hit.x > pos.x - 1.0f)
							{
								if (obj->m_npPlaceable[3] == 0)
								{
									m_pBuildingObject->m_nIsBuildingObject = 1;
									obj->m_nSelectIndex = 3;
								}
								m_pBuildingObject->SetRotation(0.0f, 90.0f, 0.0f);
								hit = Vector3::Add(pos, XMFLOAT3(0.0f, 0.5f, -2.375f));
							}
							/*	else
								{
									hit = XMFLOAT3(0.0f, 1000.0f, 0.0f);
								}*/
							maxDistance = distance;

							//hit = m_ppHierarchicalGameObjects[i]->GetPosition();
						}
					}
				}
			}

			if (m_pPlayer->m_pPlayerInventory.Rock < 2 || m_pPlayer->m_pPlayerInventory.wood <2)
			{
				m_pBuildingObject->m_nIsBuildingObject = 2;
			}
			m_pBuildingObject->SetPosition(hit);

			break;
		}
		case EObjectTag::Ceiling:
		{

			XMFLOAT3 hit = XMFLOAT3(0.0f, 1000.0f, 0.0f);
			CWallObject* obj;

			XMVECTOR origin = XMLoadFloat3(&camPos);
			XMVECTOR dir = XMLoadFloat3(&camDir);

			for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				//LogPlanter이면서 충돌이 활성화되어있는 오브젝트
				if (m_ppHierarchicalGameObjects[i]->GetType() == EObjectModelType::Wall &&
					m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision)
				{
					//Ray와 OBB 충돌 체크
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(origin, dir, distance))
					{
						XMFLOAT3 pos = m_ppHierarchicalGameObjects[i]->GetPosition();
						if (distance < maxDistance) //너무 멀리있으면 작동하지 않도록
						{
							XMVECTOR hitPos = origin + dir * distance;
							XMStoreFloat3(&hit, hitPos);
							m_pBuildingObject->m_pHitGameObject = m_ppHierarchicalGameObjects[i];
							m_pBuildingObject->m_nIsBuildingObject = 2;
							obj = (CWallObject*)m_pBuildingObject->m_pHitGameObject;
							obj->m_nSelectIndex = -1;
							//y
							if (hit.y > pos.y + 1.5f && hit.y < pos.y + 3.0f)
							{
								XMFLOAT3 dirToPlayer = Vector3::Subtract(m_pPlayer->GetPosition(), pos);
								XMFLOAT3 cilingPos = XMFLOAT3(0.0f, 3.0f, 0.0f);
								float absX = fabs(dirToPlayer.x);
								float absZ = fabs(dirToPlayer.z);
								m_pBuildingObject->m_nIsBuildingObject = 1;
								if (absX > absZ)
								{
									if (dirToPlayer.x > 0.0f)
									{

										cilingPos.x = 2.5f;   // +X
									}
									else
									{
										cilingPos.x = -2.5f;   // -X
									}
								}
								else
								{
									if (dirToPlayer.z > 0.0f)
									{
										cilingPos.z = 2.5f;   // +Z
									}
									else
									{
										cilingPos.z = -2.5f;   // -Z
									}
								}

								//m_pBuildingObject->SetRotation(0.0f, 0.0f, 0.0f);
								hit = Vector3::Add(pos, cilingPos);
							}
							maxDistance = distance;
						}
					}
				}
			}
			if (m_pPlayer->m_pPlayerInventory.wood < 4)
			{
				m_pBuildingObject->m_nIsBuildingObject = 2;
			}
			m_pBuildingObject->SetPosition(hit);
			break;
		}
		case EObjectTag::Environment: //설치가능한 환경오브젝트는 사실 밭말곤 없음
		{
			XMFLOAT3 hit = XMFLOAT3(0.0f, 1000.0f, 0.0f);

			XMVECTOR origin = XMLoadFloat3(&camPos);
			XMVECTOR dir = XMLoadFloat3(&camDir);


			for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				//LogPlanter이면서 충돌이 활성화되어있는 오브젝트
				if (m_ppHierarchicalGameObjects[i]->GetType() == EObjectModelType::Floor && m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision)
				{
					//Ray와 OBB 충돌 체크
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(origin, dir, distance))
					{
						if (distance < maxDistance) //너무 멀리있으면 작동하지 않도록
						{
							XMVECTOR hitPos = origin + dir * distance;
							XMStoreFloat3(&hit, hitPos);
							//m_pBuildingObject->SetPosition(hit);
							maxDistance = distance;
							m_pBuildingObject->m_pHitGameObject = m_ppHierarchicalGameObjects[i];
							m_pBuildingObject->m_nIsBuildingObject = 1;
							//hit = m_ppHierarchicalGameObjects[i]->GetPosition();
						}
					}
				}
				//Floor가 아니라 다른 무언가랑 충돌한 경우
				if(m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision && m_ppHierarchicalGameObjects[i]->GetType() != EObjectModelType::Floor)
				{
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(m_pBuildingObject->m_obbBox))
					{
						m_pBuildingObject->m_nIsBuildingObject = 2;
					}
				}
			}

			if (m_pPlayer->m_pPlayerInventory.Rock < 1 || m_pPlayer->m_pPlayerInventory.wood < 1)
			{
				m_pBuildingObject->m_nIsBuildingObject = 2;
			}

			m_pBuildingObject->SetPosition(hit);

			break;
		}
		case EObjectTag::Crop: //작물인 경우
		{
			XMFLOAT3 hit = XMFLOAT3(0.0f, 1000.0f, 0.0f);

			XMVECTOR origin = XMLoadFloat3(&camPos);
			XMVECTOR dir = XMLoadFloat3(&camDir);


			for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			{
				// 이 부분 서버가 동기화를 안해둬서 흠.. 그냥 넘어갈까요?? 네네 선생님 TODO::
				//LogPlanter이면서 충돌이 활성화되어있는 오브젝트
				if (m_ppHierarchicalGameObjects[i]->GetType() == EObjectModelType::LogPlanter && m_ppHierarchicalGameObjects[i]->m_bIsBoxCollision)
				{
					//Ray와 OBB 충돌 체크
					if (m_ppHierarchicalGameObjects[i]->m_obbBox.Intersects(origin, dir, distance))
					{
						if (distance < maxDistance) //너무 멀리있으면 작동하지 않도록
						{
							//m_pBuildingObject->SetPosition(hit);
							maxDistance = distance;
							m_pBuildingObject->m_pHitGameObject = m_ppHierarchicalGameObjects[i];
							m_pBuildingObject->m_nIsBuildingObject = 1;
							hit = m_ppHierarchicalGameObjects[i]->GetPosition();

							for (int j = 0; j < m_nHierarchicalGameObjects; j++)
							{
								if (m_ppHierarchicalGameObjects[j]->m_obbBox.Intersects(m_pBuildingObject->m_obbBox) &&
									m_ppHierarchicalGameObjects[j]->GetTag() == EObjectTag::Crop)
								{
									m_pBuildingObject->m_nIsBuildingObject = 2;
									break;
								}
							}

							/*    if (dynamic_cast<CLogPlnaterObject*>(m_ppHierarchicalGameObjects[i])->m_cropState == ECropState::Empty)
								{
									m_pBuildingObject->m_nIsBuildingObject = 1;
								}*/

						}
					}
				}
			}

			switch (m_pBuildingObject->m_objType)
			{
			case EObjectModelType::CornSeed:
			{
				if (m_pPlayer->m_pPlayerInventory.coonSeed < 1)
				{
					m_pBuildingObject->m_nIsBuildingObject = 2;
				}
				break;
			}

			case EObjectModelType::PotatoSeed:
			{
				if (m_pPlayer->m_pPlayerInventory.potatoSeed < 1)
				{
					m_pBuildingObject->m_nIsBuildingObject = 2;
				}
				break;
			}

			case EObjectModelType::PumpkinSeed:
			{
				if (m_pPlayer->m_pPlayerInventory.pumpkinSeed < 1)
				{
					m_pBuildingObject->m_nIsBuildingObject = 2;
				}
				break;
			}

			case EObjectModelType::TomatoSeed:
			{
				if (m_pPlayer->m_pPlayerInventory.tomatoSeed < 1)
				{
					m_pBuildingObject->m_nIsBuildingObject = 2;
				}
				break;
			}

			default:
				break;
			}


			m_pBuildingObject->SetPosition(hit);
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

void CMainScene::PlaceObject()
{
	if (m_pBuildingObject->GetActivie() && m_pBuildingObject->m_nIsBuildingObject == 1)
	{
		switch (m_pBuildingObject->m_objType)
		{
		case EObjectModelType::LogPlanter:
		{
			CGameObject* pCGameObject =
				SpawnDefaultObject(m_LogPlnaterObjectPool, m_pLogPlanterModel, EObjectTag::Environment, MakeTempId());

			pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
			pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());


			SendPlaceReq(pCGameObject->GetID(), EObjectModelType::LogPlanter, pCGameObject->GetPosition(), pCGameObject->GetQuaternion());

			m_pPlayer->m_pPlayerInventory.wood -= 1 , m_pPlayer->m_pPlayerInventory.Rock -=1;
			break;
		}

		case EObjectModelType::Floor:
		{
			CGameObject* pCGameObject = SpawnDefaultObject(m_FloorObjectPool, m_pfloorModel, EObjectTag::Floor, MakeTempId());
			pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
			pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());
			SendPlaceReq(pCGameObject->GetID(), EObjectModelType::Floor, pCGameObject->GetPosition(), pCGameObject->GetQuaternion());
			m_pPlayer->m_pPlayerInventory.Rock -= 3;
			break;
		}
		case EObjectModelType::Ceiling:
		{
			if (m_pBuildingObject->m_pHitGameObject)
			{
				CCeilingObject* obj = (CCeilingObject*)m_pBuildingObject->m_pHitGameObject;
				CGameObject* pCGameObject = SpawnDefaultObject(m_CeilingObjectPool, m_pceilingModel, EObjectTag::Ceiling, MakeTempId());
				pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());

				SendPlaceReq(pCGameObject->GetID(), EObjectModelType::Ceiling, pCGameObject->GetPosition(), pCGameObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.wood -= 4;
			}
			break;
		}
		case EObjectModelType::Wall:
		{
			if (m_pBuildingObject->m_pHitGameObject)
			{
				CFloorObject* obj = (CFloorObject*)m_pBuildingObject->m_pHitGameObject;
				obj->m_npPlaceable[obj->m_nSelectIndex] = 1;
				CGameObject* pCGameObject = SpawnDefaultObject(m_WallObjectPool, m_pWallModel, EObjectTag::Wall, MakeTempId());
				if (obj->m_nSelectIndex == 1 || obj->m_nSelectIndex == 3)
				{
					pCGameObject->Rotate(0.0f, 90.0f, 0.0f);
				}
				pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());

				m_pBuildingObject->SetRotation(0.0f, 0.0f, 0.0f);
				SendPlaceReq(pCGameObject->GetID(), EObjectModelType::Wall, pCGameObject->GetPosition(), pCGameObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.Rock -= 2, m_pPlayer->m_pPlayerInventory.wood -= 3;
			}
			break;
		}
		case EObjectModelType::CornSeed:
		{
			// 현재 충돌 되어 있는게 있다면 ==> 작물은 Hit 판정이 Log 기준
			if (m_pBuildingObject->m_pHitGameObject)
			{
				dynamic_cast<CLogPlnaterObject*>(m_pBuildingObject->m_pHitGameObject)->m_cropState = ECropState::Growing;

		/*		CGameObject* pCGameObject = SpawnDefaultObject(m_CornObjectPool, m_pCornSeedModel, EObjectTag::Crop, MakeTempId());

				pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());*/
		
				SendPlaceReq(MakeTempId(), EObjectModelType::CornSeed, m_pBuildingObject->GetPosition(), m_pBuildingObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.coonSeed -= 1;
			}
			break;
		}

		case EObjectModelType::PotatoSeed:
		{
			// 현재 충돌 되어 있는게 있다면 ==> 작물은 Hit 판정이 Log 기준
			if (m_pBuildingObject->m_pHitGameObject)
			{
				dynamic_cast<CLogPlnaterObject*>(m_pBuildingObject->m_pHitGameObject)->m_cropState = ECropState::Growing;

				//CGameObject* pCGameObject = SpawnDefaultObject(m_PotatoObjectPool, m_pPotatoSeedModel, EObjectTag::Crop, MakeTempId());
				//pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				//pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());
	
				SendPlaceReq(MakeTempId(), EObjectModelType::PotatoSeed, m_pBuildingObject->GetPosition(), m_pBuildingObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.potatoSeed -= 1;
			}
			break;
		}

		case EObjectModelType::TomatoSeed:
		{
			// 현재 충돌 되어 있는게 있다면 ==> 작물은 Hit 판정이 Log 기준
			if (m_pBuildingObject->m_pHitGameObject)
			{
				dynamic_cast<CLogPlnaterObject*>(m_pBuildingObject->m_pHitGameObject)->m_cropState = ECropState::Growing;

				//CGameObject* pCGameObject = SpawnDefaultObject(m_TomatoObjectPool, m_pTomatoSeedModel, EObjectTag::Crop, MakeTempId());
				//pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				//pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());

				SendPlaceReq(MakeTempId(), EObjectModelType::TomatoSeed, m_pBuildingObject->GetPosition(), m_pBuildingObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.tomatoSeed -= 1;
			}
			break;
		}

		case EObjectModelType::PumpkinSeed:
		{
			// 현재 충돌 되어 있는게 있다면 ==> 작물은 Hit 판정이 Log 기준
			if (m_pBuildingObject->m_pHitGameObject)
			{
				dynamic_cast<CLogPlnaterObject*>(m_pBuildingObject->m_pHitGameObject)->m_cropState = ECropState::Growing;

		/*		CGameObject* pCGameObject = SpawnDefaultObject(m_PumpkinObjectPool, m_pPumpkinSeedModel, EObjectTag::Crop, MakeTempId());
				pCGameObject->SetPosition(m_pBuildingObject->GetPosition());
				pCGameObject->SetQuaternion(m_pBuildingObject->GetQuaternion());
				pCGameObject->SetID(MakeTempId());*/
				SendPlaceReq(MakeTempId(), EObjectModelType::PumpkinSeed, m_pBuildingObject->GetPosition(), m_pBuildingObject->GetQuaternion());
				m_pPlayer->m_pPlayerInventory.pumpkinSeed -= 1;
			}
			break;
		}

		default:
		{
			break;
		}
		}
	}
}


void CMainScene::PlayerTriggerObjectUpdate()
{
	
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		CGameObject* pObj = m_ppHierarchicalGameObjects[i];
		EObjectModelType type = pObj->GetType();

		bool bCheckTarget = false;

		switch (type)
		{
			// Tree
		case EObjectModelType::Tree01:
		case EObjectModelType::Tree02:

			// Rock
		case EObjectModelType::Rock01:
		case EObjectModelType::Rock02:

			// Grass
		case EObjectModelType::Grass01:
		case EObjectModelType::Grass02:

			// Corn (03 단계)
		case EObjectModelType::Corn03:

			// Potato (03 단계)
		case EObjectModelType::Potato03:

			// Pumpkin (03 단계)
		case EObjectModelType::Pumpkin03:

			// Tomato (03 단계)
		case EObjectModelType::Tomato03:
			bCheckTarget = true;
			break;

		default:
			break;
		}

		// 대상 타입 아니면 스킵
		if (!bCheckTarget)
		{
			m_pPlayer->SetTriggerObject(NULL);
			continue;
		}

		// Collision / Trigger 조건
		if (!(pObj->m_bIsBoxCollision || pObj->m_bIsBoxTriggerCollision))
		{
			m_pPlayer->SetTriggerObject(NULL);
			continue;
		}

		// 일반 OBB  Trigger OBB 충돌 검사
		if (pObj->m_obbBox.Intersects(m_pPlayer->GetTriggerObbBox()))
		{
			m_pPlayer->SetTriggerObject(pObj);
			break;
		}
	}
}

void CMainScene::PlayerTriggerAction()
{
	CGameObject* pTarget = m_pPlayer->GetTriggerObject();
	int animationClip = 3;                 // 기본 Punch
	float animationSpeed = 1.0f;
	EAnimationState state = EAnimationState::punch;

	// 타겟이 있을 경우만 분기
	if (pTarget)
	{
		EObjectModelType type = pTarget->GetType();

		switch (type)
		{
			// Grass / Crop → Pickup
		case EObjectModelType::Grass01:
		case EObjectModelType::Grass02:
		case EObjectModelType::Corn03:
		case EObjectModelType::Potato03:
		case EObjectModelType::Pumpkin03:
		case EObjectModelType::Tomato03:
		{
			animationClip = 2;                 // Pickup
			animationSpeed = 3.0f;
			state = EAnimationState::pickup;
			//전송하려고 만듬

			break;
		}

		// Tree / Rock → Punch (그대로)
		case EObjectModelType::Tree01:
		case EObjectModelType::Tree02:
		case EObjectModelType::Rock01:
		case EObjectModelType::Rock02:
			// 기본 Punch 유지
			break;
		default:
			// 알 수 없는 타입 → 그냥 Punch
			break;
		}
	}


	// 애니메이션 실행 (공통)
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(1, true);

	m_pPlayer->m_pSkinnedAnimationController->SetTrackSpeed(1, animationSpeed);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackAnimationClip(1, animationClip);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(
		1, -ANIMATION_CALLBACK_EPSILON
	);

	// 현재 AnimationClip
	CAnimationClip* pAnimationClip =
		m_pPlayer->m_pSkinnedAnimationController
		->m_pAnimationClips
		->m_pAnimationClips[
			m_pPlayer->m_pSkinnedAnimationController
				->m_pAnimationTracks[1].m_nAnimationClip
		];

	// 종료 콜백 (Idle 복귀용)
	m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(
		1, 0, pAnimationClip->m_fLength, (void*)(uintptr_t)state
	);

	// 액션 타이밍 콜백
	switch (animationClip)
	{
	case 2: // Pickup
		m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(
			1, 1, pAnimationClip->m_fLength - 0.3f, m_pPlayer
		);
		break;

	case 3: // Punch
		m_pPlayer->m_pSkinnedAnimationController->SetCallbackKey(
			1, 1, pAnimationClip->m_fLength - 0.45f, m_pPlayer
		);
		break;
	}
	SendObjectMoveToServer(m_pPlayer, (Protocol::PlayerState)state);
	//////
}
void CMainScene::TriggerObjectDestory()
{
	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		CGameObject* pObj = m_ppHierarchicalGameObjects[i];
		

		if (!pObj) continue;

		EObjectModelType type = pObj->GetType();
		
		switch (type)
		{

			// Crop 계열

		case EObjectModelType::Corn03:
		{
			CCornObject* p = dynamic_cast<CCornObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_CornObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.coonSeed += 1;
	
				
			}
			break;
		}
		case EObjectModelType::Potato03:
		{
			CPotatoObject* p = dynamic_cast<CPotatoObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{

				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_PotatoObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.potatoSeed += 1;
		
			}
			break;
		}
		case EObjectModelType::Pumpkin03:
		{
			CPumpkinObject* p = dynamic_cast<CPumpkinObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_PumpkinObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.pumpkinSeed += 1;
		
			}
			break;
		}
		case EObjectModelType::Tomato03:
		{
			CTomatoObject* p = dynamic_cast<CTomatoObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_TomatoObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.tomatoSeed += 1;
	
			}
			break;
		}


		// Grass
	
		case EObjectModelType::Grass01:
		case EObjectModelType::Grass02:
		{
			CGrassObject* p = dynamic_cast<CGrassObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_GrassObjectPool, pObj);
				i--;
				switch (m_nSeasonIndex)
				{
				case 0:
				{
					m_pPlayer->m_pPlayerInventory.coonSeed += 3;
					break;
				}
				case 1:
				{
					m_pPlayer->m_pPlayerInventory.tomatoSeed += 3;
					break;
				}
				case 2:
				{
					m_pPlayer->m_pPlayerInventory.pumpkinSeed += 3;
					break;
				}
				case 3:
				{
					m_pPlayer->m_pPlayerInventory.potatoSeed += 3;
					break;
				}
				default:
					break;
				}
			}
			break;
		}


		// Tree
	
		case EObjectModelType::Tree01:
		case EObjectModelType::Tree02:
		{
			CTreeObject* p = dynamic_cast<CTreeObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_TreeObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.wood += 5;
			}
			break;
		}


		// Rock
		case EObjectModelType::Rock01:
		case EObjectModelType::Rock02:
		{
			CRockObject* p = dynamic_cast<CRockObject*>(pObj);
			if (p && p->GetHP() <= 0)
			{
				Protocol::C_GATHER_REQ pkt;
				pkt.set_object_id(pObj->GetID());
				auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
				m_upClientNetwork->SendToServer(sendBuffer);

				DeSpawnDefaultObject(m_RockObjectPool, pObj);
				i--;
				m_pPlayer->m_pPlayerInventory.Rock += 5;
			}
			break;
		}

		default:
			break;
		}
		
	}
}

 EObjectModelType CMainScene::StringToModelType(int name)
{
	if (name == 630 )   return EObjectModelType::Tree01;
	if (name == 631 )   return EObjectModelType::Tree02;

	if (name == 610)   return EObjectModelType::Rock01;
	if (name == 611)   return EObjectModelType::Rock02;
			 
	if (name == 640)  return EObjectModelType::Grass01;
	if (name == 641)  return EObjectModelType::Grass02;
			 
	if (name == 200)   return EObjectModelType::Corn01;
	if (name == 201)   return EObjectModelType::Corn02;
	if (name == 202)   return EObjectModelType::Corn03;
	if (name == 203)   return EObjectModelType::Corn04;
	if (name == 210) return EObjectModelType::CornSeed;
	// 필요하면 계속 추가
	return EObjectModelType::None;
}

 CLoadedModelInfo* CMainScene::GetModelInfoByType(EObjectModelType type)
 {
	 switch (type)
	 {
	 case EObjectModelType::Tree01:    return m_pTree01Model;

	 case EObjectModelType::Rock01:    return m_pRock01Model;
	 case EObjectModelType::Rock02:    return m_pRock02Model;

	 case EObjectModelType::Grass01:   return m_pGrass01Model;
	 case EObjectModelType::Grass02:   return m_pGrass02Model;

	 case EObjectModelType::Corn01:    return m_pCorn01Model;
	 case EObjectModelType::Corn02:    return m_pCorn02Model;
	 case EObjectModelType::Corn03:    return m_pCorn03Model;
	 case EObjectModelType::Corn04:    return m_pCorn04Model;
	 case EObjectModelType::CornSeed:  return m_pCornSeedModel;

	 }

	 return nullptr;
 }

 //void CMainScene::ParseMultiModelBlock(
	// ifstream& InFile,
	// StageSpawnData& data,
	// const char* endTag)
 //{
	// char token[64];

	// while (InFile >> token)
	// {
	//	 if (!strcmp(token, endTag))
	//		 break;

	//	 if (!strcmp(token, "<Model>"))
	//	 {
	//		 StageSpawnEntry entry;

	//		 // Model
	//		 char modelName[64];
	//		 InFile >> modelName;
	//		 entry.type = StringToModelType(modelName);
	//		 entry.model = GetModelInfoByType(entry.type);

	//		 while (InFile >> token)
	//		 {
	//			 if (!strcmp(token, "</Model>"))
	//				 continue;

	//			 else if (!strcmp(token, "<Count>"))
	//			 {
	//				 int count;
	//				 InFile >> count;
	//			 }
	//			 else if (!strcmp(token, "</Count>"))
	//			 {
	//				 continue;
	//			 }
	//			 else if (!strcmp(token, "<Positions>"))
	//			 {
	//				 while (InFile >> token)
	//				 {
	//					 if (!strcmp(token, "</Positions>"))
	//						 break;

	//					 float x = (float)atof(token);
	//					 float z;
	//					 InFile >> z;
	//					 entry.positions.emplace_back(x, z);
	//				 }
	//			 }
	//			 else if (!strcmp(token, "<Model>") || !strcmp(token, endTag))
	//			 {
	//				 // 다음 블록으로 되돌림
	//				 InFile.seekg(-static_cast<int>(strlen(token)), ios::cur);
	//				 break;
	//			 }
	//		 }

	//		 data.entries.push_back(entry);
	//	 }
	// }
 //}

 /*void CMainScene::ParseSingleModelBlock(
	 ifstream& InFile,
	 StageSpawnData& data,
	 const char* endTag)
 {
	 StageSpawnEntry entry;
	 char token[64];

	 while (InFile >> token)
	 {
		 if (!strcmp(token, endTag))
			 break;

		 if (!strcmp(token, "<Model>"))
		 {
			 char modelName[64];
			 InFile >> modelName;
			 entry.type = StringToModelType(modelName);
			 entry.model = GetModelInfoByType(entry.type);
		 }
		 else if (!strcmp(token, "</Model>"))
		 {
			 continue;
		 }
		 else if (!strcmp(token, "<Count>"))
		 {
			 int count;
			 InFile >> count;
		 }
		 else if (!strcmp(token, "</Count>"))
		 {
			 continue;
		 }
		 else if (!strcmp(token, "<Positions>"))
		 {
			 while (InFile >> token)
			 {
				 if (!strcmp(token, "</Positions>"))
					 break;

				 float x = (float)atof(token);
				 float z;
				 InFile >> z;
				 entry.positions.emplace_back(x, z);
			 }
		 }
	 }

	 data.entries.push_back(entry);
 }*/

 StageSpawnData CMainScene::LoadStageResourceFromFile(Protocol::S_WORLD_INIT& pkt)
{
	 StageSpawnData data;
    
	for (auto p : pkt.objects())
	{
		StageSpawnEntry spawndt;
		spawndt.positions.emplace_back(p.transform().px(), p.transform().pz());
		spawndt.type = StringToModelType(static_cast<int>(p.model_type()));
		spawndt.model = GetModelInfoByType(spawndt.type);
		spawndt.temp_id = p.temp_id();
		data.entries.emplace_back(spawndt);
	}
	return data;
}

 /*extern bool g_hasPendingWorldInit;*/
 extern std::vector<Protocol::S_WORLD_INIT> g_pendingWorldInits;

 void CMainScene::ApplyPendingWorldInit()
 {
	 std::cout << "=== ApplyPendingWorldInit ENTER ===" << std::endl;
	 std::cout << "pending count = " << g_pendingWorldInits.size() << std::endl;

	 if (g_pendingWorldInits.empty())
		 return;

	 Protocol::S_WORLD_INIT& lastPkt = g_pendingWorldInits.back();
	 std::cout << "Applying FINAL pkt objects = " << lastPkt.objects_size() << std::endl;

	 StageSpawnData data = LoadStageResourceFromFile(lastPkt);
	 std::cout << "Converted spawn data size = " << sizeof(data) << std::endl;

	 SpawnStageFromData(data);

	 g_pendingWorldInits.clear();
	 g_hasPendingWorldInit = false;

	 std::cout << "=== ApplyPendingWorldInit EXIT ===" << std::endl;
 }
void CMainScene::SpawnStageFromData(const StageSpawnData& data)
{
	for (auto& entry : data.entries)
	{
		for (auto& pos : entry.positions)
		{
			float y = m_pTerrain->GetHeight(
				pos.x + 249.0f,
				pos.y + 249.0f
			);

			CGameObject* obj = nullptr;

			switch (entry.type)
			{
			case EObjectModelType::Tree01:
				obj = SpawnDefaultObject(
					m_TreeObjectPool,
					entry.model,
					EObjectTag::Resource,
					entry.temp_id
				);
				break;

			case EObjectModelType::Rock01:
			case EObjectModelType::Rock02:
				obj = SpawnDefaultObject(
					m_RockObjectPool,
					entry.model,
					EObjectTag::Resource,
					entry.temp_id
				);
				break;

			case EObjectModelType::Grass01:
			case EObjectModelType::Grass02:
				obj = SpawnDefaultObject(
					m_GrassObjectPool,
					entry.model,
					EObjectTag::Resource,
					entry.temp_id
				);
				break;
			}

			if (obj)
			{
				obj->SetPosition(XMFLOAT3(pos.x, y, pos.y));
				WorldSpawnInfo info;
				info.objId = obj->GetID();
				info.type = static_cast<uint32>(entry.type);
				info.positions = XMFLOAT3(pos.x, y, pos.y);
				toSend.push_back(info);
			}
		}
	}
	SendWorldInitToServer(toSend);
	//toSend.clear();
}

///////////////////////////////////////////

CGameObject* CMainScene::FindObjectByServerID(uint64 id)
{

	if (m_pPlayer && m_pPlayer->GetID() == id)
		return m_pPlayer;

	for (int i = 0; i < m_nHierarchicalGameObjects; ++i)
	{
		CGameObject* obj = m_ppHierarchicalGameObjects[i];
		if (!obj) continue;
		if (obj->GetID() == id)
			return obj;
	}
	return nullptr;
}

void CMainScene::SendPlaceReq(int id,EObjectModelType type, const XMFLOAT3& pos, XMFLOAT4& qpos)
{
	Protocol::C_PLACEOBJECTREQ req;
	req.set_req_id(id);
	req.set_model_type(static_cast<uint32>(type));
	req.mutable_transform()->set_px(pos.x);
	req.mutable_transform()->set_py(pos.y);
	req.mutable_transform()->set_pz(pos.z);
	req.mutable_transform()->set_qx(qpos.x);
	req.mutable_transform()->set_qy(qpos.y);
	req.mutable_transform()->set_qz(qpos.z);
	req.mutable_transform()->set_qw(qpos.w);
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(req);
	m_upClientNetwork->SendToServer(sendBuffer);
}
void CMainScene::SendWorldInitToServer(const std::vector<WorldSpawnInfo>& list)
{
	static std::vector<WorldSpawnInfo> sAccum;
	sAccum.reserve(10000);
	static int sCallCount = 0;
	static bool sSent = false;

	if (sSent)
		return;

	// 1) 이번 호출분 누적
	sAccum.insert(sAccum.end(), list.begin(), list.end());
	sCallCount++;

	std::cout << "[WorldInit] chunk received. call=" << sCallCount
		<< " chunkSize=" << list.size()
		<< " accumSize=" << sAccum.size() << "\n";

	//// 2) 아직 3번 다 안 모였으면 전송하지 않음
	if (sCallCount < 3)
		return;

	//// 3) 여기부터 "딱 한 번만" 전송
	//Protocol::C_WORLD_INIT pkt;

	//for (const auto& s : list)
	//{
	//	auto* o = pkt.add_objects();
	//	o->set_temp_id(s.objId);
	//	o->set_model_type(s.type);
	//	o->mutable_transform()->set_px(s.positions.x);
	//	o->mutable_transform()->set_py(s.positions.y);
	//	o->mutable_transform()->set_pz(s.positions.z);
	//}
	//auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	//m_upClientNetwork->SendToServer(sendBuffer);

	std::cout << "[WorldInit] SENT once. total=" << sAccum.size() << "\n";

	// 4) 플래그 처리
	sSent = true;
	sAccum.clear();
}
