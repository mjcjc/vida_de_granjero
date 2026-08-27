#include "pch.h"
#include "ServerThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"
#include "DBConfig.h"
#include"DBManager.h"
#include"GameLogic.h"
#include "GameLogicGlobal.h"
#include "WorldEnviroment.h"
#include "GlobalWorld.h"
#include "WorldObjectDB.h"
#include "ServerNavigation.h"

WorldEnvironment GWorldEnv;
GlobalWorld GWorld;

int main()
{
	ASSERT_CRASH(GNavigation.Init());

	DBConfig cfg;
	cfg.SetHost("tcp://127.0.0.1:3306");
	cfg.SetUser("game_user");
	cfg.SetPass("StrongPassword!");
	cfg.SetSchema("game_db");
	cfg.SetPoolSize(16);

	ASSERT_CRASH(DBManager::Init(cfg, 4));
	CreateWorldObjectTableIfNeeded();

	ClientPacketHandler::Init();
	auto gameLogic = std::make_unique<GameLogic>(&GWorldEnv);
	GGameLogic = gameLogic.get();
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, 
		100);
	
	ASSERT_CRASH(service->Start());

		GThreadManager->Launch([]()
			{
				while (true)
				{
					uint64 nowMs = ::GetTickCount64();
					GWorld.UpdateGrowth(nowMs, ESeason::Autumn, EWeather::Clear);
					this_thread::sleep_for(std::chrono::seconds(1));
				}
			});

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([service]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}				
			});
	}	

	GThreadManager->Join();
}
