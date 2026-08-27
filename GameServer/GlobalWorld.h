#pragma once
#include "pch.h"
#include <unordered_map>
#include "WorldEnviroment.h"
#include "ServerNavigation.h"

using ObjectId = uint64;

struct WorldObject
{
    ObjectId id = 0;
    uint32 modelType = 0;
    float x = 0, y = 0, z = 0;
	float qx = 0, qy = 0, qz = 0, qw = 1;
    uint64 ownerPlayerId = 0;
    uint32 hp = 100;
    bool alive = true;
    uint64 tempId = 0;
    float growth = 0.0f;          
    uint64 lastGrowMs = 0;        
    uint8 growthStage = 0;
    bool init = false;
    
};

class GlobalWorld
{
public:
    ObjectId ClSpawn(uint64 tempid, uint32 modelType, uint32 hp, float x, float y, float z, float qx, float qy, float qz, float qw, uint64 ownerPlayerId);

    ObjectId Spawn(uint64 tempid,uint32 modelType, uint32 hp,float x, float y, float z, float qx, float qy, float qz, float qw, uint64 ownerPlayerId);
    std::vector<WorldObject> Snapshot() const;
    std::vector<NavigationObstacle> GetNavigationObstacles() const;

    void SendSnapshotTo(PacketSessionRef& session);
    int GetInitialized() const { return _initialized; }
    bool MarkInitialized(bool _initialized) {  return _initialized; }
    bool Despawn(ObjectId objectId);
    bool PlayerDespawn(uint64 objectId);
    bool ApplySeedGrowthRule(uint64 objectId, bool hasCeiling);
    std::unordered_map<ObjectId, WorldObject> _objects;
    bool SetModelType(uint64 objectId, uint32 newModelType);
    int GetSpawnIdGen() { return _idGen++; };
    bool SetModelhp(uint64 objectId, uint64 hp);
    void UpdateGrowth(uint64 nowMs, ESeason season, EWeather weather);
    void ReadTextFile(char* pstrFileName, PacketSessionRef & session);
    uint32 StringToModelType(char* name);
    void ParseMultiModelBlock(ifstream& InFile, const char* endTag, PacketSessionRef& session);
    void ParseSingleModelBlock(ifstream& InFile, const char* endTag, PacketSessionRef& session);
    bool LoadFromDB();
private:

    bool IsCropDamaged = false;
    bool _initialized = 0;
    mutable Lock _locks[1];           
    Atomic<ObjectId> _idGen = 1000;
};

extern GlobalWorld GWorld;
