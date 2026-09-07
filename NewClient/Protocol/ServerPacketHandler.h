#pragma once
#include "CorePch.h"
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
	PKT_C_DEBUG_WORLD_STATE_CHANGE = 1010,
	PKT_S_WORLD_INIT = 1011,
	PKT_C_GATHER_REQ = 1012,
	PKT_C_OBEJCT_HP = 1013,
	PKT_S_OBJECT_DESPAWN = 1014,
	PKT_S_SEEDGROWTH = 1015,
	PKT_C_SEEDGROWTH = 1016,
	PKT_C_CROPSTATE = 1017,
	PKT_S_PLAYER_DESPAWN = 1018,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt);
bool Handle_S_OTHER(PacketSessionRef& session, Protocol::S_OTHER& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt);
bool Handle_S_OBJECTSPAWN(PacketSessionRef& session, Protocol::S_OBJECTSPAWN& pkt);
bool Handle_S_WORLD_STATE(PacketSessionRef& session, Protocol::S_WORLD_STATE& pkt);
bool Handle_S_WORLD_INIT(PacketSessionRef& session, Protocol::S_WORLD_INIT& pkt);
bool Handle_S_OBJECT_DESPAWN(PacketSessionRef& session, Protocol::S_OBJECT_DESPAWN& pkt);
bool Handle_S_SEEDGROWTH(PacketSessionRef& session, Protocol::S_SEEDGROWTH& pkt);
bool Handle_S_PLAYER_DESPAWN(PacketSessionRef& session, Protocol::S_PLAYER_DESPAWN& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ENTER_GAME>(Handle_S_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_S_OTHER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OTHER>(Handle_S_OTHER, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE>(Handle_S_MOVE, session, buffer, len); };
		GPacketHandler[PKT_S_OBJECTSPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJECTSPAWN>(Handle_S_OBJECTSPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_WORLD_STATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_WORLD_STATE>(Handle_S_WORLD_STATE, session, buffer, len); };
		GPacketHandler[PKT_S_WORLD_INIT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_WORLD_INIT>(Handle_S_WORLD_INIT, session, buffer, len); };
		GPacketHandler[PKT_S_OBJECT_DESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJECT_DESPAWN>(Handle_S_OBJECT_DESPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_SEEDGROWTH] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_SEEDGROWTH>(Handle_S_SEEDGROWTH, session, buffer, len); };
		GPacketHandler[PKT_S_PLAYER_DESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_PLAYER_DESPAWN>(Handle_S_PLAYER_DESPAWN, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		if (buffer == nullptr || len < static_cast<int32>(sizeof(PacketHeader))) return false;
		PacketHeader header{};
		::memcpy(&header, buffer, sizeof(header));
		if (header.size != len) return false;
		return GPacketHandler[header.id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PLACEOBJECTREQ& pkt) { return MakeSendBuffer(pkt, PKT_C_PLACEOBJECTREQ); }
	static SendBufferRef MakeSendBuffer(Protocol::C_DEBUG_WORLD_STATE_CHANGE& pkt) { return MakeSendBuffer(pkt, PKT_C_DEBUG_WORLD_STATE_CHANGE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_GATHER_REQ& pkt) { return MakeSendBuffer(pkt, PKT_C_GATHER_REQ); }
	static SendBufferRef MakeSendBuffer(Protocol::C_OBEJCT_HP& pkt) { return MakeSendBuffer(pkt, PKT_C_OBEJCT_HP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_SEEDGROWTH& pkt) { return MakeSendBuffer(pkt, PKT_C_SEEDGROWTH); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CROPSTATE& pkt) { return MakeSendBuffer(pkt, PKT_C_CROPSTATE); }

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
		PacketHeader header{};
		header.size = packetSize;
		header.id = pktId;
		::memcpy(sendBuffer->Buffer(), &header, sizeof(header));
		ASSERT_CRASH(pkt.SerializeToArray(sendBuffer->Buffer() + sizeof(PacketHeader), dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};