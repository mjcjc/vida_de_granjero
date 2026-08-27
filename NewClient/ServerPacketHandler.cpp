#include "pch.h"
#include "ServerPacketHandler.h"
#include "../Scene.h"
#include "../GameFramework.h"
#include "../MainScene.h"
#include"../StartScene.h"

extern CGameFramework gGameFramework;
class CGameObject;

class CScene;
class CStartScene;
PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 직접 컨텐츠 작업자
UINT GMyServerPlayerId = 0;
bool StartFlagForServer = false;
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader header{};
	if (buffer != nullptr && len >= static_cast<int32>(sizeof(PacketHeader)))
		::memcpy(&header, buffer, sizeof(header));
	// TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{

	if (pkt.success() == false)
		return true;

	if (pkt.players().size() == 0)
	{
		// 캐릭터 생성창
	}
	GMyServerPlayerId = pkt.players(0).id();
	// 입장 UI 버튼 눌러서 게임 입장

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	CScene* base = CSceneManager::GetInstance().GetTopScene();
	auto* scene = dynamic_cast<CMainScene*>(base);
	if (!scene) return true;

	uint64 id = pkt.playerid();

	// 1) 내 ID가 아직 없으면 "내 ID 후보"로 먼저 잡는다
	//    (대부분 서버가 내 세션에 먼저 보내므로 잘 맞음)
	if (GMyServerPlayerId == 0)
	{
		GMyServerPlayerId = id;

		if (scene->m_pPlayer)
			scene->m_pPlayer->SetID(GMyServerPlayerId);

		return true;
	}

	// 2) 내 ID면 스폰 금지
	if (id == GMyServerPlayerId)
		return true;

	// 3) 이미 있으면 스킵
	for (int i = 0; i < scene->m_nHierarchicalGameObjects; i++)
	{
		if (scene->m_ppHierarchicalGameObjects[i] &&
			scene->m_ppHierarchicalGameObjects[i]->GetID() == id)
			return true;
	}

	// 4) 다른 사람 스폰
	scene->SpawnAnimateObject(
		scene->m_WoodManObjectPool,
		scene->m_pWoodManModel,
		EObjectTag::Player,
		id,
		2);

	return true;
}
/*int count = 0;

		cout << pkt.success() << endl;

		CScene* base = CSceneManager::GetInstance().GetTopScene();

		auto* scene = dynamic_cast<CMainScene*>(base);

		if (scene)
		{

			scene->m_pPlayer->SetID(GMyServerPlayerId);

			cout << "player" << pkt.playerid() << endl;
			for (count = 0; count < scene->m_nHierarchicalGameObjects; count++)
			{
				if (scene->m_ppHierarchicalGameObjects[count]->GetID() == pkt.playerid() && pkt.playerid() != GMyServerPlayerId)
				{
					break;
				}
			}
			if (count == scene->m_nHierarchicalGameObjects)
			{
				CGameObject* obj = scene->SpawnAnimateObject(
					scene->m_AnimatedObjectPool,
					scene->m_pWoodManModel,
					EObjectTag::Player,
					pkt.playerid(),
					2);

			}
		}
		return true;*/
//bool Handle_S_OTHER(PacketSessionRef& session, Protocol::S_OTHER& pkt)
//{
//	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
//	if (!scene) return true;
//
//	for (int i = 0; i < pkt.other_size(); ++i)
//	{
//		const auto& p = pkt.other(i);
//		uint64 serverId = p.id();
//
//		// 이미 있으면 스킵
//		if (scene->FindObjectByServerID(serverId))
//			continue;
//
//		CGameObject* obj = scene->SpawnAnimateObject(
//			scene->m_AnimatedObjectPool,
//			scene->m_pWoodManModel,
//			EObjectTag::Player,
//			scene->m_nIDPool++,   // 로컬ID는 별도로
//			2);
//
//		obj->SetID(serverId);          // 서버ID는 따로 저장
//		//obj->ApplyProtoTransform(p.transform());
//	}
//	return true;
//}
bool Handle_S_OTHER(PacketSessionRef& session, Protocol::S_OTHER& pkt)
{
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	if (!scene) return true;
	//cout << "다른 클라 수" << pkt.other_size() << endl;
	for (int i = 0; i < pkt.other_size(); ++i)
	{
		const auto& p = pkt.other(i);
		uint64 serverId = p.id();
		//cout << "다른 클라 아이디" << serverId << endl;
		if (scene->FindObjectByServerID(serverId))
			continue;

		CGameObject* obj = scene->SpawnAnimateObject(
			scene->m_WoodManObjectPool,
			scene->m_pWoodManModel,
			EObjectTag::Player,
			serverId,
			2);
	
		const auto& t = p.transform();
		obj->SetPosition(t.px(), t.py(), t.pz());
		obj->SetQuaternion(XMFLOAT4(t.qx(), t.qy(), t.qz(), t.qw()));

		// (선택) 즉시 월드 갱신이 필요하면:
		// obj->UpdateTransform(nullptr);  // Shader 구조에 따라 필요/불필요
	}
	return true;
}
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt)
{
	/*cout << "클라 번호" << GMyServerPlayerId << endl;
	cout << "이건 서버" << pkt.playerid() << endl;*/
	/*std::cout << "[CLIENT] S_MOVE recv id=" << pkt.playerid()
		<< " pos=(" << pkt.player().transform().px() << ", "
		<< pkt.player().transform().py() << ", "
		<< pkt.player().transform().pz() << ")"
		<< " fpos=(" << pkt.player().transform().fx() << ", "
		<< pkt.player().transform().fy() << ", "
		<< pkt.player().transform().fz() << ")"
		<< " rpos=(" << pkt.player().transform().rx() << ", "
		<< pkt.player().transform().ry() << ", "
		<< pkt.player().transform().rz() << ")"
		<< " upos=(" << pkt.player().transform().ux() << ", "
		<< pkt.player().transform().uy() << ", "
		<< pkt.player().transform().uz() << ")" << std::endl;*/
	CScene* base = CSceneManager::GetInstance().GetTopScene();
	auto* scene = dynamic_cast<CMainScene*>(base);
	if (!scene) return true;

	if (pkt.playerid() == GMyServerPlayerId)
	{
		if (!scene->m_pPlayer)
			return true;

		const auto& serverTransform = pkt.player().transform();
		const XMFLOAT3 authoritativePosition(
			serverTransform.px(), serverTransform.py(), serverTransform.pz());
		scene->ReconcileLocalPlayer(scene->m_pPlayer, authoritativePosition,
			pkt.last_processed_sequence());
		return true;
	}

	CGameObject* target = scene->FindObjectByServerID(pkt.playerid());
	if (!target) return true;

	const auto& t = pkt.player().transform();
	RemoteMovementSnapshot snapshot;
	snapshot.arrivalTimeMs = ::GetTickCount64();
	snapshot.serverTick = pkt.server_tick();
	snapshot.position = XMFLOAT3(t.px(), t.py(), t.pz());
	snapshot.rotation = XMFLOAT4(t.qx(), t.qy(), t.qz(), t.qw());
	snapshot.state = pkt.player().state();
	scene->QueueRemoteMovementSnapshot(pkt.playerid(), snapshot);

	return true;
}

// S_MOVE: 특정 오브젝트의 Transform 업데이트
//bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt)
//{
//	cout << "클라 번호" << GMyServerPlayerId << endl;
//	cout << "이건 서버" << pkt.playerid() << endl;
//	std::cout << "[CLIENT] S_MOVE recv id=" << pkt.playerid()
//		<< " pos=(" << pkt.player().transform().px() << ", "
//		<< pkt.player().transform().py() << ", "
//		<< pkt.player().transform().pz() << ")"
//		<< " fpos=(" << pkt.player().transform().fx() << ", "
//		<< pkt.player().transform().fy() << ", "
//		<< pkt.player().transform().fz() << ")"
//		<< " rpos=(" << pkt.player().transform().rx() << ", "
//		<< pkt.player().transform().ry() << ", "
//		<< pkt.player().transform().rz() << ")"
//		<< " upos=(" << pkt.player().transform().ux() << ", "
//		<< pkt.player().transform().uy() << ", "
//		<< pkt.player().transform().uz() << ")" << std::endl;
//
//
//	CScene* base = CSceneManager::GetInstance().GetTopScene();
//	auto* scene = dynamic_cast<CMainScene*>(base);
//	if (!scene) return true;
//
//	CGameObject* target = scene->FindObjectByServerID(pkt.playerid());
//
//	if (target)
//	{
//
//		if (pkt.playerid() != GMyServerPlayerId)
//		{
//			//target->ApplyProtoTransform(pkt.player().transform());
//
//			return true;
//		}
//		std::cout << "[CLIENT] SpawnNetworkPlayer id=" << pkt.playerid() << std::endl;
//	}
//	return true;
//
//}

bool Handle_S_OBJECTSPAWN(PacketSessionRef& session, Protocol::S_OBJECTSPAWN& pkt)
{
	//std::cout << "[Handle_S_OBJECTSPAWN] id=" << pkt.object_id()
	//	<< " tempId=" << pkt.temp_id()
	//	<< " pos=(" << pkt.transform().px() << "," << pkt.transform().py() << "," << pkt.transform().pz() << ")\n";
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	if (!scene) {
		//std::cout << "[Handle_S_OBJECTSPAWN] scene null. top scene is not MainScene\n";
		return true;
	}

	uint64 objId = pkt.object_id();

	//WRITE_LOCK

	// 이미 있으면 중복 스폰 방지
	if (scene->FindObjectByServerID(objId))
	{
		return true;
	}
	//cout << "오브젝트 스폰 타입" << pkt.model_type() << endl;
	// model_type에 따라 어떤 모델/풀을 쓸지 선택
	CLoadedModelInfo* model = nullptr;
	EObjectTag tag = EObjectTag::Environment;
	CGameObject* obj = nullptr;

	switch (pkt.model_type())
	{
	case 210:
		obj = scene->SpawnDefaultObject(scene->m_CornObjectPool, scene->m_pCornSeedModel, EObjectTag::Crop, objId);
		break;
	case 310:
		obj = scene->SpawnDefaultObject(scene->m_PotatoObjectPool, scene->m_pPotatoSeedModel, EObjectTag::Crop, objId);
		break;
	case 410:
		obj = scene->SpawnDefaultObject(scene->m_PumpkinObjectPool, scene->m_pPumpkinSeedModel, EObjectTag::Crop, objId);
		break;
	case 510:
		obj = scene->SpawnDefaultObject(scene->m_TomatoObjectPool, scene->m_pTomatoSeedModel, EObjectTag::Crop, objId);
		break;
	case 600:
		obj = scene->SpawnDefaultObject(scene->m_LogPlnaterObjectPool, scene->m_pLogPlanterModel, EObjectTag::Environment, objId);
		break;
	case 601:
		obj = scene->SpawnDefaultObject(scene->m_BarnObjectPool, scene->m_pBarnModel, EObjectTag::Environment, objId);
		break;
	case   610:
		obj = scene->SpawnDefaultObject(scene->m_RockObjectPool, scene->m_pRock01Model, EObjectTag::Resource, objId);
		break;
	case 611:
		obj = scene->SpawnDefaultObject(scene->m_RockObjectPool, scene->m_pRock02Model, EObjectTag::Resource, objId);
		break;
	case 620:
		obj = scene->SpawnDefaultObject(scene->m_WallObjectPool, scene->m_pWallModel, EObjectTag::Wall, objId);
		break;
	case 621:
		obj = scene->SpawnDefaultObject(scene->m_CeilingObjectPool, scene->m_pceilingModel, EObjectTag::Ceiling, objId);
		break;
	case 622:
		obj = scene->SpawnDefaultObject(scene->m_FloorObjectPool, scene->m_pfloorModel, EObjectTag::Floor, objId);
		break;
	case 630:
		obj = scene->SpawnDefaultObject(scene->m_TreeObjectPool, scene->m_pTree01Model, EObjectTag::Resource, objId);
		break;
	case 631:
		obj = scene->SpawnDefaultObject(scene->m_TreeObjectPool, scene->m_pTree01Model, EObjectTag::Resource, objId);
		break;
	case 640:
		obj = scene->SpawnDefaultObject(scene->m_GrassObjectPool, scene->m_pGrass01Model, EObjectTag::Resource, objId);
		break;
	case 641:
		obj = scene->SpawnDefaultObject(scene->m_GrassObjectPool, scene->m_pGrass02Model, EObjectTag::Resource, objId);
		break;
	default:
		return true;
	}
	/*if (model == nullptr)
	{
		std::cout << "[S_OBJECTSPAWN] FAIL: model null. type=" << pkt.model_type() << "\n";
		return true;
	}*/
	if (obj==nullptr)
	{
		//std::cout << "[S_OBJECTSPAWN] FAIL: model null. type=" << pkt.model_type() << "\n";
		return true;
	}
	obj->SetPosition(pkt.transform().px(), pkt.transform().py(), pkt.transform().pz());
	obj->SetQuaternion(XMFLOAT4(pkt.transform().qx(), pkt.transform().qy(), pkt.transform().qz(), pkt.transform().qw()));

	//cout << "오브젝트 위치 설정 완료" << endl;
	return true;
}

bool Handle_S_WORLD_STATE(PacketSessionRef& session, Protocol::S_WORLD_STATE& pkt)
{
	//std::cout << "[Client] S_WORLD_STATE "
	//	<< "season=" << pkt.season()
	//	<< " weather=" << pkt.weather()
	//	<< " score=" << pkt.mission_score()
	//	<< "\n";
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	scene->m_nWeather = pkt.weather();
	scene->m_nSeasonIndex = pkt.season();
	scene->m_fScore = pkt.mission_score();
	return true;
}

static bool g_hasPendingWorldInit = false;
std::vector<Protocol::S_WORLD_INIT> g_pendingWorldInits;

bool Handle_S_WORLD_INIT(PacketSessionRef& session, Protocol::S_WORLD_INIT& pkt)
{
	std::cout << "S_WORLD_INIT received, objects = " << pkt.objects_size() << std::endl;

	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());

	if (!scene)
	{
		std::cout << "MainScene not ready, pending push" << std::endl;
		g_pendingWorldInits.push_back(pkt);
		return true;
	}

	StageSpawnData data = scene->LoadStageResourceFromFile(pkt);
	scene->SpawnStageFromData(data);
	return true;
}


bool Handle_S_OBJECT_DESPAWN(PacketSessionRef& session, Protocol::S_OBJECT_DESPAWN& pkt)
{
	//std::cout << "[Handle_S_OBJECT_DESPAWN] id=" << pkt.object_id() ;
	//std::cout << " 지금 hp " << pkt.hp()<<endl;
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	if (!scene) {
		//std::cout << "[Handle_S_OBJECT_DESPAWN] scene null. top scene is not MainScene\n";
		return true;
	}

	uint64 objId = pkt.object_id();
	CGameObject* obj = scene->FindObjectByServerID(objId);
	if (!obj)
	{
		return true;
	}

	switch (int(obj->GetType()))
	{

	case 202:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp()<<endl;
		CCropObject* cobj = dynamic_cast<CCropObject*>(obj);
		cobj->SetHP(pkt.hp());
		//cout << "hp : " << cobj->GetHP() << endl;
		break;
	}
	case 302:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp() << endl;
		CPotatoObject* cobj = dynamic_cast<CPotatoObject*>(obj);
		cobj->SetHP(pkt.hp());

		//cout << "hp : " << cobj->GetHP() << endl;
			break;
	}
	case 402:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp() << endl;
		CPumpkinObject* cobj = dynamic_cast<CPumpkinObject*>(obj);
		cobj->SetHP(pkt.hp());

		//cout << "hp : " << cobj->GetHP() << endl;
			break;

	}
	case 502:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp() << endl;
		CTomatoObject* cobj = dynamic_cast<CTomatoObject*>(obj);
		cobj->SetHP(pkt.hp());

		//cout << "hp : " << cobj->GetHP() << endl;
		break;
	}
	case 610:
	case 611:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp() << endl;
		auto* cobj = dynamic_cast<CRockObject*>(obj);
		cobj->SetHP(pkt.hp());

		//cout << "hp : " << cobj->GetHP() << endl;
		break;

	}
	case 630:
	case 631:
	{
		//cout << " Id : " << pkt.object_id();
		//cout << "hp : " << pkt.hp() << endl;
		auto* cobj = dynamic_cast<CTreeObject*>(obj);
		cobj->SetHP(pkt.hp());

		//cout << "hp : " << cobj->GetHP() << endl;
			break;
	}
	case 640:
	case 641:
	{

		auto* cobj = dynamic_cast<CGrassObject*>(obj);
		cobj->SetHP(pkt.hp());	

		cout << "hp : " << cobj->GetHP() << endl;
		break;
	}
	
	}
	
	return true;
}
bool Handle_S_SEEDGROWTH(PacketSessionRef& session, Protocol::S_SEEDGROWTH& pkt)
{
	//cout << pkt.growth() << " " << pkt.object_id() << endl;
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	uint64 objId = pkt.object_id();
	CGameObject* obj = scene->FindObjectByServerID(objId);
	if (!obj) return true;
	//cout << "X값: " << obj->GetPosition().x << " Y값: " << obj->GetPosition().y << " Z값: " << obj->GetPosition().z << endl;
	
	if (auto* crop = dynamic_cast<CCropObject*>(obj))
	{
		crop->SetGrowth(pkt.growth());
		
		//std::cout << "[CLIENT] Seed growth update id=" << objId << " growth=" << pkt.growth() << "\n";
	}

	return true;
}
bool Handle_S_PLAYER_DESPAWN(PacketSessionRef& session, Protocol::S_PLAYER_DESPAWN& pkt)
{
	auto* scene = dynamic_cast<CMainScene*>(CSceneManager::GetInstance().GetTopScene());
	CGameObject* obj = scene->FindObjectByServerID(pkt.playerid());
	if (!obj) return true;
	//cout << obj->GetID() << endl;
	scene->DeSpawnAnimateObject(scene->m_WoodManObjectPool, obj);

	return true;
}
