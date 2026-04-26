#pragma once
#include "pch.h"
#include "ServerThreadManager.h"
#include "ClientSession.h"
#include "Service.h"

class ClientNetwork
{
private:
    bool _connected = false;
public:
    bool IsConnected() const { return _connected; }
    void SetConnected(bool v) { _connected = v; }

public:
    // ip, port로 접속 시작
    bool Start(const wchar_t* ip, uint16 port);

    // 전체 세션 브로드캐스트 (그냥 편의 함수)
    void Broadcast(SendBufferRef sendBuffer);
    void SendToServer(SendBufferRef sendBuffer);
	
    ClientServiceRef GetService() const { return _service; }
	SessionRef GetSession() const { return _session; }

private:
    ClientServiceRef _service;
	SessionRef _session;
};
