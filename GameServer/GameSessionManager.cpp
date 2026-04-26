#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include <ServerThreadManager.h>
#include <DeadLockProfiler.h>
#include <Service.h>

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK; 
	auto it = std::find(_sessions.begin(), _sessions.end(), session);
	if (it != _sessions.end())
		_sessions.erase(it);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}