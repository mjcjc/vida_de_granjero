#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "ServerPlayer.h"
#include "GlobalWorld.h"

static std::atomic<int> GSessionIdGen = 100000;

GameSession::GameSession() 
{
	_sessionId = GSessionIdGen.fetch_add(1); 
}

void GameSession::OnConnected()
{
    auto self = static_pointer_cast<GameSession>(shared_from_this());
    GSessionManager.Add(self);
    std::cout << "[GameSession] OnConnected" << std::endl;

    _currentPlayer = MakeShared<Player>();
    _currentPlayer->playerId = _sessionId;

    _currentPlayer->ownerSession = static_pointer_cast<GameSession>(shared_from_this());
	cout << "[GameSession] New player connected with ID " << _currentPlayer->playerId << endl;
    // 방/매니저에 등록
  
    Protocol::S_LOGIN pkt;              // 이름만 로그인일 뿐, 지금은 단순 ID통지용으로 사용

    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    Send(sendBuffer);                   // 이 세션에만 전송
}


void GameSession::OnDisconnected()
{
    uint64 pid = _currentPlayer->playerId;
    if (pid != 0)
    {

        GWorld.PlayerDespawn(pid);

        Protocol::S_PLAYER_DESPAWN pkt;
        pkt.set_playerid(pid);

        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

        GSessionManager.ForEachSession([&](SessionRef s)
            {
            
                if (s.get() == this) return;
                s->Send(sendBuffer);
            });
    }
    GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}
