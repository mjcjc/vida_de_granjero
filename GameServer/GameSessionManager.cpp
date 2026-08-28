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
	if (session->_accountDbId != 0) {
		auto accountIt = _accountSessions.find(session->_accountDbId);
		if (accountIt != _accountSessions.end() && accountIt->second == session) {
			_accountSessions.erase(accountIt);
		}
		session->_accountDbId = 0;
	}

	auto it = std::find(_sessions.begin(), _sessions.end(), session);
	if (it != _sessions.end())
		_sessions.erase(it);
}

bool GameSessionManager::TryBindAccount(uint64 accountDbId, const GameSessionRef& session) {
	WRITE_LOCK;
	if (accountDbId == 0 || session == nullptr || session->_accountDbId != 0) {
		return false;
	}

	if (_accountSessions.find(accountDbId) != _accountSessions.end()) {
		return false;
	}

	_accountSessions.emplace(accountDbId, session);
	session->_accountDbId = accountDbId;
	return true;
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}
