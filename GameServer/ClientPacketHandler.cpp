#include "pch.h"
#include "ClientPacketHandler.h"
#include "ServerPlayer.h"
#include "Room.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include"GameLogic.h"
#include "GlobalWorld.h"
#include"GameLogicGlobal.h"
#include"WorldObjectDB.h"
#include "ServerNavigation.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 직접 컨텐츠 작업자

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader header{};
	if (buffer != nullptr && len >= static_cast<int32>(sizeof(PacketHeader)))
		::memcpy(&header, buffer, sizeof(header));
	
	//std::cout << "[SERVER][INVALID] id=" << header->id << " size=" << header->size << std::endl;

	return false;
}
//보낼때 count,
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (pkt.accountid().empty() || pkt.password().empty())
		return false;

	//cout << "[Handle_C_LOGIN] Login request received for player id: " << pkt.accountid() << " "<<pkt.password() <<endl;
	// TODO : Validation 체크
	CreateWorldObjectTableIfNeeded();

	if (IsWorldObjectTableEmpty())
	{
		cout << "[WorldInit] DB empty -> load from file" << endl;
		GWorld.ReadTextFile(const_cast<char*>("..\\Model\\Spawn\\Stage01.txt"), session);
		GWorld.ReadTextFile(const_cast<char*>("..\\Model\\Spawn\\Stage02.txt"), session);
		GWorld.ReadTextFile(const_cast<char*>("..\\Model\\Spawn\\Stage03.txt"), session);
	}
	else
	{
		cout << "[WorldInit] DB exists -> load from DB" << endl;
		GWorld.LoadFromDB();
	}	Protocol::S_LOGIN loginPkt;
	{
		PlayerRef playerRef = gameSession->_currentPlayer; 
		auto player = loginPkt.add_players();
		player->set_id(playerRef->playerId);
		player->set_playertype(Protocol::PLAYER_TYPE_NONE);
		player->mutable_transform()->set_px(0.f);
		player->mutable_transform()->set_py(0.f);
		player->mutable_transform()->set_pz(0.f);
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.push_back(playerRef);
		//cout << "[Handle_C_LOGIN] Player logged in: " << playerRef->name << " with ID " << playerRef->playerId << endl;
		
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		session->Send(sendBuffer);

		return true;
	}
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	GWorld.SendSnapshotTo(session);
	 uint64 index = pkt.playerindex();
	// TODO : Validation
	 PlayerRef myPlayer = gameSession->_currentPlayer;
	// cout << "[Handle_C_ENTER_GAME] Player entering game: " << myPlayer->name << " with ID " << myPlayer->playerId << endl;
	Protocol::S_ENTER_GAME enterGamePkt;
	enterGamePkt.set_success(true);
	enterGamePkt.set_playerid(myPlayer->playerId);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	GSessionManager.Broadcast(sendBuffer);
	GWorldEnv.SendCurrentStateToClient(gameSession.get());
	Protocol::S_OTHER otherP;
	otherP.set_p_size(static_cast<uint32>(gameSession->_players.size()));
	//cout << "현재 접속자 수" << otherP.p_size() << endl;
	GSessionManager.ForEachSession([&](GameSessionRef s) {
		PlayerRef pl = s->_currentPlayer;
		if (!pl) return true;

		// 자기 자신은 제외(선택)
		if (pl->playerId == myPlayer->playerId)
			return true;

		auto* msg = otherP.add_other();
		msg->set_id(pl->playerId);
		msg->set_state(pl->playerState);

		auto& ts = pl->ts;
		auto* tr = msg->mutable_transform();
		tr->set_qx(ts.qx());
		tr->set_qy(ts.qy());
		tr->set_qz(ts.qz());
		tr->set_qw(ts.qw());
		tr->set_px(ts.px());
		tr->set_py(ts.py());
		tr->set_pz(ts.pz());
		}
	);
		/*for (int i = 0; i < gameSession->_players.size(); i++) {
			PlayerRef pl = gameSession->_players[i];
			if (!pl) continue;
			auto msg = otherP.add_other();
			msg->set_id(pl->playerId);
			msg->set_state(pl->playerState);
			
			auto& ts = pl->ts;
			auto* tr = msg->mutable_transform();
			tr->set_fx(ts.fx());
			tr->set_fy(ts.fy());
			tr->set_fz(ts.fz());
			tr->set_px(ts.px());
			tr->set_py(ts.py());
			tr->set_pz(ts.pz());
			tr->set_rx(ts.rx());
			tr->set_ry(ts.ry());
			tr->set_rz(ts.rz());
			tr->set_ux(ts.ux());
			tr->set_uy(ts.uy());
			tr->set_uz(ts.uz());
		}*/
	
	otherP.set_p_size(otherP.p_size());
	auto sendBuffer2 = ClientPacketHandler::MakeSendBuffer(otherP);
	session->Send(sendBuffer2);
//	cout << "[S_OTHER][SERVER] 보낸다 현재 접속자 수" << otherP.p_size() << endl;

	//auto snap = GWorld.Snapshot();
	//for (const auto& o : snap)
	//{
	//	Protocol::S_OBJECTSPAWN spawnPkt;
	//	spawnPkt.set_object_id(o.id);
	//	spawnPkt.set_model_type(o.modelType);
	//	spawnPkt.mutable_transform()->set_qx(o.qx);
	//	spawnPkt.mutable_transform()->set_qy(o.qy);
	//	spawnPkt.mutable_transform()->set_qz(o.qz);
	//	spawnPkt.mutable_transform()->set_qw(o.qw);
	//	spawnPkt.mutable_transform()->set_px(o.x);
	//	spawnPkt.mutable_transform()->set_py(o.y);
	//	spawnPkt.mutable_transform()->set_pz(o.z);

	//	spawnPkt.set_owner_player_id(o.ownerPlayerId);

	//	session->Send(ClientPacketHandler::MakeSendBuffer(spawnPkt));
	//}

	return true;
}



bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef myPlayer = gameSession->_currentPlayer;

	if (myPlayer == nullptr || !pkt.has_player() || !pkt.player().has_transform())
		return false;
	if (pkt.sequence() == 0 || pkt.sequence() <= myPlayer->lastProcessedMoveSequence)
		return true;

	const uint64 nowMs = ::GetTickCount64();
	float elapsedSeconds = 0.05f;
	if (myPlayer->lastMoveTimeMs != 0)
		elapsedSeconds = static_cast<float>(nowMs - myPlayer->lastMoveTimeMs) / 1000.0f;
	myPlayer->lastMoveTimeMs = nowMs;

	const Protocol::Transform& requestedTransform = pkt.player().transform();
	const ServerPosition current{ myPlayer->ts.px(), myPlayer->ts.py(), myPlayer->ts.pz() };
	float inputX = pkt.input_x();
	float inputZ = pkt.input_z();
	if (!std::isfinite(inputX) || !std::isfinite(inputZ))
	{
		inputX = 0.0f;
		inputZ = 0.0f;
		++myPlayer->invalidMoveCount;
	}
	const float inputLength = std::sqrt(inputX * inputX + inputZ * inputZ);
	if (inputLength > 1.0f)
	{
		inputX /= inputLength;
		inputZ /= inputLength;
	}
	const float clientDelta = std::clamp(pkt.delta_time(), 0.001f, 0.1f);
	const float simulationDelta = (std::min)(clientDelta, std::clamp(elapsedSeconds, 0.001f, 0.1f));
	constexpr float movementSpeed = 6.0f;
	const ServerPosition requested{
		current.x + inputX * movementSpeed * simulationDelta,
		requestedTransform.py(),
		current.z + inputZ * movementSpeed * simulationDelta };
	std::vector<NavigationObstacle> obstacles = GWorld.GetNavigationObstacles();
	const float playerRadius = ServerNavigation::GetPlayerRadius();
	GSessionManager.ForEachSession([&](const GameSessionRef& otherSession)
	{
		const PlayerRef& otherPlayer = otherSession->_currentPlayer;
		if (otherPlayer == nullptr || otherPlayer->playerId == myPlayer->playerId)
			return;

		const float otherX = otherPlayer->ts.px();
		const float otherZ = otherPlayer->ts.pz();
		obstacles.push_back(NavigationObstacle{
			otherPlayer->playerId,
			otherX - playerRadius, otherX + playerRadius,
			otherZ - playerRadius, otherZ + playerRadius });
	});
	const MoveValidationResult validation = GNavigation.ValidateMove(
		current, requested, simulationDelta, obstacles);

	if (validation.corrected)
	{
		++myPlayer->invalidMoveCount;
		std::cout << "[MOVE_CORRECTION] playerId=" << myPlayer->playerId
			<< " reason=" << static_cast<int32>(validation.reason)
			<< " requested=(" << requested.x << ", " << requested.y << ", " << requested.z << ")"
			<< " corrected=(" << validation.position.x << ", " << validation.position.y
			<< ", " << validation.position.z << ") count=" << myPlayer->invalidMoveCount << std::endl;
	}
	else if (myPlayer->invalidMoveCount > 0)
	{
		--myPlayer->invalidMoveCount;
	}

	myPlayer->ts.CopyFrom(requestedTransform);
	myPlayer->ts.set_px(validation.position.x);
	myPlayer->ts.set_py(validation.position.y);
	myPlayer->ts.set_pz(validation.position.z);
	myPlayer->playerState = pkt.player().state();
	myPlayer->lastProcessedMoveSequence = pkt.sequence();

	Protocol::S_MOVE movePkt;
	movePkt.set_playerid(myPlayer->playerId);
	movePkt.mutable_player()->set_id(myPlayer->playerId);
	movePkt.mutable_player()->set_state(myPlayer->playerState);
	movePkt.mutable_player()->mutable_transform()->CopyFrom(myPlayer->ts);
	movePkt.set_last_processed_sequence(myPlayer->lastProcessedMoveSequence);
	movePkt.set_server_tick(static_cast<uint32>(nowMs));

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
	GSessionManager.Broadcast(sendBuffer);

	if (myPlayer->invalidMoveCount >= 20)
		std::cout << "[MOVE_ABUSE] playerId=" << myPlayer->playerId << std::endl;

	return true;
}
bool Handle_C_PLACEOBJECTREQ(PacketSessionRef& session, Protocol::C_PLACEOBJECTREQ& pkt)
{
	//cout << "[Handle_C_PLACEOBJECTREQ] Received place object request" << endl;
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef myPlayer = gameSession->_currentPlayer;
	if (!myPlayer)
		return true;
	//cout << "[Handle_C_PLACEOBJECTREQ] Player placing object: " << myPlayer->name << " with ID " << myPlayer->playerId << endl;
	// 1) 간단 검증(최소)
	uint32 modelType = pkt.model_type();
	if (modelType == 0) // None = 0 같은 거 막기
		return true;
	//cout << "[Handle_C_PLACEOBJECTREQ] modelType: " << modelType << endl;
	// 2) 서버 권위로 오브젝트 생성(전역맵 1개)
	Protocol::S_OBJECTSPAWN spawnPkt;
	if ((modelType > 200 && modelType <= 600)||(modelType > 619 && modelType <= 623))
	{
		ObjectId newId = GWorld.Spawn(
			pkt.req_id(),
			modelType,
			100,
			pkt.transform().px(),
			pkt.transform().py(), pkt.transform().pz(), pkt.transform().qx(),
			pkt.transform().qy(), pkt.transform().qz(), pkt.transform().qw(),
			0
		);
		
		spawnPkt.set_object_id(newId);
		spawnPkt.set_temp_id(pkt.req_id());
		spawnPkt.set_model_type(modelType);
		spawnPkt.mutable_transform()->set_qx(pkt.transform().qx());
		spawnPkt.mutable_transform()->set_qy(pkt.transform().qy());
		spawnPkt.mutable_transform()->set_qz(pkt.transform().qz());
		spawnPkt.mutable_transform()->set_qw(pkt.transform().qw());
		spawnPkt.mutable_transform()->set_px(pkt.transform().px());
		spawnPkt.mutable_transform()->set_py(pkt.transform().py());
		spawnPkt.mutable_transform()->set_pz(pkt.transform().pz());

	}
	else {
		ObjectId newId = GWorld.ClSpawn(
			pkt.req_id(),
			modelType,
			100,
			pkt.transform().px(),
			pkt.transform().py(), pkt.transform().pz(), pkt.transform().qx(),
			pkt.transform().qy(), pkt.transform().qz(), pkt.transform().qw(),
			0
		);
		Protocol::S_OBJECTSPAWN spawnPkt;
		spawnPkt.set_object_id(pkt.req_id());
		spawnPkt.set_temp_id(pkt.req_id());
		spawnPkt.set_model_type(modelType);
		spawnPkt.mutable_transform()->set_qx(pkt.transform().qx());
		spawnPkt.mutable_transform()->set_qy(pkt.transform().qy());
		spawnPkt.mutable_transform()->set_qz(pkt.transform().qz());
		spawnPkt.mutable_transform()->set_qw(pkt.transform().qw());
		spawnPkt.mutable_transform()->set_px(pkt.transform().px());
		spawnPkt.mutable_transform()->set_py(pkt.transform().py());
		spawnPkt.mutable_transform()->set_pz(pkt.transform().pz());
	}
	// 3) 모든 클라에게 스폰 브로드캐스트
	

	spawnPkt.set_owner_player_id(myPlayer->playerId);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
	//cout << "[SERVER] sending packet id=" << header->id
	//	<< " size=" << header->size << endl;
	GSessionManager.Broadcast(sendBuffer);

	//cout << "[Handle_C_PLACEOBJECTREQ] Broadcasting object spawn: ID " << spawnPkt.object_id() << ", modelType " << modelType << endl;
	return true;
}

//bool Handle_C_WORLD_INIT(PacketSessionRef& session, Protocol::C_WORLD_INIT& pkt)
//{
//	if (GWorld.GetInitialized())
//	{
//	//	std::cout << "[C_WORLD_INIT] ignored (already initialized)\n";
//		return true;
//	}
//
//	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
//	if (!gameSession)
//		return true;
//
//	uint64 senderId = gameSession->_currentPlayer->playerId;
//	//std::cout << "[C_WORLD_INIT] from player " << senderId
//	//	<< " count=" << pkt.objects_size() << "\n";
//
//	// 2) 서버 월드에 스폰
//	//for (int i = 0; i < pkt.objects_size(); i++)
//	//{
//	//	const Protocol::WorldObject& obj = pkt.objects(i);
//	//	const Protocol::Transform& t = obj.transform();
//
//	//	GWorld.ClSpawn(
//	//		obj.temp_id(),
//	//		obj.model_type(),
//	//		100,
//	//		t.px(), t.py(), t.pz(),
//	//		t.qx(), t.qy(), t.qz(), t.qw(),
//	//		0 // 월드 리소스는 owner 없음
//	//	);
//	//	cout << "[C_WORLD_INIT] object spawned: "<<obj.temp_id()
//	//		<< obj.model_type()
//	//		<< " pos=(" << t.px() << ", " << t.py() << ", " << t.pz() <<")\n";
//	//}
//
//	// 3) 월드 초기화 완료 마킹
//	GWorld.MarkInitialized(true);
//
//	// 4) 서버에 있는 월드 전체를 모든 클라에게 스폰 브로드캐스트
//	// (이미 있는 S_OBJECTSPAWN 재사용)
//	auto snap = GWorld.Snapshot();
//	for (const auto& o : snap)
//	{
//		Protocol::S_OBJECTSPAWN spawnPkt;
//		spawnPkt.set_object_id(o.ownerPlayerId);
//		spawnPkt.set_model_type(o.modelType);
//
//		spawnPkt.mutable_transform()->set_px(o.x);
//		spawnPkt.mutable_transform()->set_py(o.y);
//		spawnPkt.mutable_transform()->set_pz(o.z);
//		spawnPkt.mutable_transform()->set_qx(o.qx);
//		spawnPkt.mutable_transform()->set_qy(o.qy);
//		spawnPkt.mutable_transform()->set_qz(o.qz);
//		spawnPkt.mutable_transform()->set_qw(o.qw);
//
//		spawnPkt.set_owner_player_id(0);
//		session->Send(ClientPacketHandler::MakeSendBuffer(spawnPkt));
//	}
//	return true;
//}


bool Handle_C_GATHER_REQ(PacketSessionRef& session, Protocol::C_GATHER_REQ& pkt)
{
	uint64 objectId = pkt.object_id();

	auto it = GWorld._objects.find(objectId);
	if (it == GWorld._objects.end())
	{
		cout << "[C_GATHER_REQ] invalid object " << objectId << endl;
		return true;
	}

	uint32 modelType = it->second.modelType;

	bool isHarvestCrop =
		(modelType == 200 || modelType == 300 || modelType == 400 || modelType == 500);

	if (isHarvestCrop)
	{
		GGameLogic->OnMissionCompleted(10);
	}

	bool removed = GWorld.Despawn(objectId);
	if (!removed)
	{
		cout << "[C_GATHER_REQ] invalid object " << objectId << endl;
		return true;
	}

	cout << "[C_GATHER_REQ] object removed: " << objectId << endl;

	Protocol::S_OBJECT_DESPAWN despawnPkt;
	despawnPkt.set_object_id(objectId);
	auto sendBuf = ClientPacketHandler::MakeSendBuffer(despawnPkt);
	GSessionManager.Broadcast(sendBuf);

	Protocol::S_WORLD_STATE pst;
	pst.set_mission_score(GWorldEnv.GetMissionScore());
	pst.set_season((int32_t)GWorldEnv.GetSeason());
	pst.set_weather((int32_t)GWorldEnv.GetWeather());
	auto sendBuffer2 = ClientPacketHandler::MakeSendBuffer(pst);
	GSessionManager.Broadcast(sendBuffer2);

	return true;
}

bool Handle_C_DEBUG_WORLD_STATE_CHANGE(
	PacketSessionRef& session,
	Protocol::C_DEBUG_WORLD_STATE_CHANGE& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	if (!gameSession || !gameSession->_currentPlayer)
		return true;

	if (pkt.season() > static_cast<uint32>(ESeason::Winter) ||
		pkt.weather() > static_cast<uint32>(EWeather::Fog))
	{
		std::cout << "[WORLD_STATE_CHANGE] rejected playerId="
			<< gameSession->_currentPlayer->playerId
			<< " season=" << pkt.season()
			<< " weather=" << pkt.weather() << std::endl;
		return true;
	}

	GWorldEnv.ChangeWorldState(
		static_cast<ESeason>(pkt.season()),
		static_cast<EWeather>(pkt.weather()));
	return true;
}


bool Handle_C_SEEDGROWTH(PacketSessionRef& session, Protocol::C_SEEDGROWTH& pkt)
{
	int64 objectId = pkt.object_id();
	uint64 check = pkt.cropcheckcell(); // 천장 체크

	// check 값 의미를 확실히: 0이면 천장 없음, 1이면 천장 있음 가정
	bool hasCeiling = check;

	bool ok = GWorld.ApplySeedGrowthRule(objectId, hasCeiling);
	if (!ok)
	{
		std::cout << "[C_SEEDGROWTH] invalid objectId=" << objectId << "\n";
	}
	return true;
}

bool Handle_C_CROPSTATE(PacketSessionRef& session, Protocol::C_CROPSTATE& pkt)
{
	const uint64 objectId = pkt.object_id();
	const uint32 newModelType = pkt.model_type(); // proto 필드명 맞춰

	if (!GWorld.SetModelType(objectId, newModelType))
	{
		std::cout << "[C_CROPSTATE] invalid objectId=" << objectId
			<< " newModelType=" << newModelType << "\n";
		return true;
	}
	return true;
}

bool Handle_C_OBEJCT_HP(PacketSessionRef& session, Protocol::C_OBEJCT_HP& pkt)
{
	uint64 objectId = pkt.object_id();
	uint64 hp = pkt.hp();
	cout << "id :" << pkt.object_id() << endl;
	if (!GWorld.SetModelhp(objectId, hp))
	{
		return true;
	}
	
	return true;
}


