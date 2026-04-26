#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ENTER_GAME = 1002,
	PKT_S_ENTER_GAME = 1003,
	PKT_S_OTHER = 1004,
	PKT_C_MOVE = 1005,
	PKT_S_MOVE = 1006,
	PKT_C_PLACEOBJECTREQ = 1007,
	PKT_S_OBJECTSPAWN = 1008,
	PKT_S_WORLD_STATE = 1009,
	PKT_S_WORLD_INIT = 1010,
	PKT_C_GATHER_REQ = 1011,
	PKT_C_OBEJCT_HP = 1012,
	PKT_S_OBJECT_DESPAWN = 1013,
	PKT_S_SEEDGROWTH = 1014,
	PKT_C_SEEDGROWTH = 1015,
	PKT_C_CROPSTATE = 1016,
	PKT_S_PLAYER_DESPAWN = 1017,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_PLACEOBJECTREQ(PacketSessionRef& session, Protocol::C_PLACEOBJECTREQ& pkt);
bool Handle_C_GATHER_REQ(PacketSessionRef& session, Protocol::C_GATHER_REQ& pkt);
bool Handle_C_OBEJCT_HP(PacketSessionRef& session, Protocol::C_OBEJCT_HP& pkt);
bool Handle_C_SEEDGROWTH(PacketSessionRef& session, Protocol::C_SEEDGROWTH& pkt);
bool Handle_C_CROPSTATE(PacketSessionRef& session, Protocol::C_CROPSTATE& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_C_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ENTER_GAME>(Handle_C_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_PLACEOBJECTREQ] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_PLACEOBJECTREQ>(Handle_C_PLACEOBJECTREQ, session, buffer, len); };
		GPacketHandler[PKT_C_GATHER_REQ] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_GATHER_REQ>(Handle_C_GATHER_REQ, session, buffer, len); };
		GPacketHandler[PKT_C_OBEJCT_HP] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_OBEJCT_HP>(Handle_C_OBEJCT_HP, session, buffer, len); };
		GPacketHandler[PKT_C_SEEDGROWTH] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_SEEDGROWTH>(Handle_C_SEEDGROWTH, session, buffer, len); };
		GPacketHandler[PKT_C_CROPSTATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CROPSTATE>(Handle_C_CROPSTATE, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, PKT_S_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OTHER& pkt) { return MakeSendBuffer(pkt, PKT_S_OTHER); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OBJECTSPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_OBJECTSPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_WORLD_STATE& pkt) { return MakeSendBuffer(pkt, PKT_S_WORLD_STATE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_WORLD_INIT& pkt) { return MakeSendBuffer(pkt, PKT_S_WORLD_INIT); }
	static SendBufferRef MakeSendBuffer(Protocol::S_OBJECT_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_OBJECT_DESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SEEDGROWTH& pkt) { return MakeSendBuffer(pkt, PKT_S_SEEDGROWTH); }
	static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_PLAYER_DESPAWN); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};