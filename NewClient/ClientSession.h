#pragma once
#include "pch.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

class ClientSession : public PacketSession
{
public:
    virtual ~ClientSession() {}



    virtual void OnConnected() override
    {
       
    }

    // 패킷 단위 수신
    virtual void OnRecvPacket(BYTE* buffer, int32 len) override
    {
        PacketSessionRef session = GetPacketSessionRef();
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

        // TODO : packetId 대역 체크
        ServerPacketHandler::HandlePacket(session, buffer, len);
    }

    // 송신 완료
    virtual void OnSend(int32 len) override
    {
        
    }

    // 연결 끊김
    virtual void OnDisconnected() override
    {
       
    }
};
