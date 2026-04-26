#pragma once
#include"Protocol.pb.h"
//namespace Protocol
//{
//    class S_WORLD_STATE; // forward declare (Protocol.pb.h include´Â cpp¿¡¼­)
//}

class IWorldBroadcaster
{
public:
    virtual ~IWorldBroadcaster() = default;
    virtual void BroadcastWorldState(const Protocol::S_WORLD_STATE& pkt) = 0;
};
