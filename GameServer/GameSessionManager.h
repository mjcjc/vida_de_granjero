#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager
{
public:
	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);
public:
	template<typename Func>
	void ForEachSession(Func&& func)
	{
		READ_LOCK;
		for (auto& s : _sessions)
			func(s);
	}

private:
	USE_LOCK;
	Set<GameSessionRef> _sessions;

};

extern GameSessionManager GSessionManager;
