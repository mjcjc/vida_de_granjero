#include "pch.h"
#include "GlobalWorld.h"
#include"Protocol.pb.h"
#include"ClientPacketHandler.h"
#include"GameSessionManager.h"
#include"GameLogic.h"
#include "GameLogicGlobal.h"
#include "fstream"
#include "WorldObjectDB.h"

    
static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}
ObjectId GlobalWorld::ClSpawn(uint64 tempid,uint32 modelType, uint32 hp, float x, float y, float z, float qx, float qy, float qz, float qw, uint64 ownerPlayerId)
{
    WRITE_LOCK; // 너 서버의 lock 매크로(= WriteLockGuard) 사용 중이면 이거 쓰면 됨
    WorldObject obj;
    obj.tempId = tempid;
    obj.hp = hp;
    obj.modelType = modelType;
    obj.x = x; obj.y = y; obj.z = z;
    obj.qx = qx;
	obj.qy = qy;
	obj.qz = qz;
	obj.qw = qw;
    obj.growth = 0.0f;
    obj.ownerPlayerId = ownerPlayerId;
    std::cout << "[SPAWN] owberPlayerId=" << ownerPlayerId
		<< " objectId=" << tempid
        << " model=" << modelType
        << " (seed planted)\n";
    _objects.emplace(tempid, obj);
    return tempid;
}
//기존은 위
ObjectId GlobalWorld::Spawn(
    uint64 tempid,
    uint32 modelType,
    uint32 hp,
    float x, float y, float z,
    float qx, float qy, float qz, float qw,
    uint64 ownerPlayerId)
{
    WRITE_LOCK;

    ObjectId id = _idGen.fetch_add(1);   

    WorldObject obj;
    obj.id = id;
    obj.tempId = tempid;                 // (ACK 매핑용, 없어도 됨)
    obj.hp = hp;
    obj.modelType = modelType;
    obj.x = x; obj.y = y; obj.z = z;
    obj.qx = qx; obj.qy = qy; obj.qz = qz; obj.qw = qw;
    obj.growth = 0.0f;
    obj.ownerPlayerId = ownerPlayerId;
    obj.alive = true;

    DBWorldObjectData dbData;
    dbData.objectId = obj.id;
    dbData.modelType = obj.modelType;
    dbData.posX = obj.x;
    dbData.posY = obj.y;
    dbData.posZ = obj.z;
    dbData.rotX = obj.qx;
    dbData.rotY = obj.qy;
    dbData.rotZ = obj.qz;
    dbData.rotW = obj.qw;
    dbData.hp = obj.hp;
    dbData.growth = obj.growth;

    UpsertWorldObjectToDB(dbData);

    std::cout << "[SPAWN] owner=" << ownerPlayerId
        << " serverId=" << id
        << " tempId=" << tempid
        << " model=" << modelType << "\n";

    _objects.emplace(id, obj);
    return id;
}


std::vector<WorldObject> GlobalWorld::Snapshot() const
{
    READ_LOCK;

    std::vector<WorldObject> out;
    out.reserve(_objects.size());
    for (auto& [id, obj] : _objects)
        out.push_back(obj);

    return out;
}
void GlobalWorld::SendSnapshotTo(PacketSessionRef& session)
{
    Protocol::S_WORLD_INIT pkt;

    for (auto& pair : _objects)
    {
        const WorldObject& it = pair.second;
        auto o = pkt.add_objects();

        o->mutable_transform()->set_px(it.x);
        o->mutable_transform()->set_py(it.y);
        o->mutable_transform()->set_pz(it.z);
        o->mutable_transform()->set_qx(it.qx);
        o->mutable_transform()->set_qy(it.qy);
        o->mutable_transform()->set_qz(it.qz);
        o->mutable_transform()->set_qw(it.qw);

        o->set_model_type(it.modelType);

        // 가능하면 temp_id 말고 object_id도 같이 보내는 구조가 더 좋음
        o->set_temp_id(it.id);
    }

    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    session->Send(sendBuffer);
}
//bool GlobalWorld::Despawn(ObjectId objectId, uint64 requestPlayerId)
//{
//    WRITE_LOCK;
//
//    auto it = _objects.find(objectId);
//    if (it == _objects.end())
//    {
//        std::cout << "[DESPAWN] not found objectId=" << objectId << "\n";
//        return false;
//    }
//
//    WorldObject& obj = it->second;
//
//    if (!obj.alive)
//    {
//        std::cout << "[DESPAWN] already dead objectId=" << objectId << "\n";
//        return false;
//    }
//    std::cout << "[DESPAWN CHECK] objId=" << objectId
//        << " storedOwner=" << obj.ownerPlayerId
//        << " requester=" << requestPlayerId << "\n";
//    if (obj.ownerPlayerId != requestPlayerId)
//    {
//        std::cout << "[DESPAWN] owner mismatch. objectId=" << objectId
//            << " owner=" << obj.ownerPlayerId
//            << " requester=" << requestPlayerId << "\n";
//        return false;
//    }
//
//    obj.hp = 0;
//    obj.alive = false;
//    if (obj.modelType == 202 || obj.modelType == 302 || obj.modelType == 402 || obj.modelType == 502)
//    {
//        GGameLogic->OnMissionCompleted(20); // 예시: 미션 점수 증가
//        Protocol::S_WORLD_STATE pkt;
//        pkt.set_mission_score(GWorldEnv.GetMissionScore());
//        pkt.set_season((int32_t)GWorldEnv.GetSeason());
//        pkt.set_weather((int32_t)GWorldEnv.GetWeather());
//        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
//        GSessionManager.Broadcast(sendBuffer);
//    }
//    std::cout << "[DESPAWN] OK objectId=" << objectId
//        << " by player=" << requestPlayerId << "\n";
//
//    return true;
//}

bool GlobalWorld::SetModelType(uint64 objectId, uint32 newModelType)
{
    WRITE_LOCK;

    auto it = _objects.find(objectId);
    if (it == _objects.end())
        return false;

    WorldObject& obj = it->second;

    if (!obj.alive)
        return false;

    obj.modelType = newModelType;
    return true;
}

bool GlobalWorld::SetModelhp(uint64 objectId, uint64 newModelType)
{
    WRITE_LOCK;

    auto it = _objects.find(objectId);
    if (it == _objects.end())
        return false;

    WorldObject& obj = it->second;

    if (!obj.alive)
        return false;

    obj.hp = newModelType;
    Protocol::S_OBJECT_DESPAWN pkt;
    pkt.set_hp(obj.hp);
    pkt.set_object_id(obj.id);
    cout << " object hp " << pkt.hp();
    cout << " id : " << pkt.object_id() << endl;
    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    GSessionManager.Broadcast(sendBuffer);
    return true;
}

bool GlobalWorld::PlayerDespawn(uint64 id)
{
    WRITE_LOCK;
    bool removed = false;

    for (auto it = _objects.begin(); it != _objects.end(); )
    {
        if (it->second.ownerPlayerId == id)
        {
            DeleteWorldObjectFromDB(it->second.ownerPlayerId);
            it = _objects.erase(it);
            removed = true;
        }
        else
        {
            ++it;
        }
    }
    return removed;
}
 bool GlobalWorld::Despawn(ObjectId objectId)
 {
    WRITE_LOCK;

   // bool removed = false;
   // auto it = _objects.find(objectId);
   // if (it == _objects.end())
   //     return false;

   // auto& obj = it->second;

   ///* if (!obj.alive)
   //     return false;*/

   // // 미션 처리
   // if (obj.modelType == 202 || obj.modelType == 302 ||
   //     obj.modelType == 402 || obj.modelType == 502)
   // {
   //     GGameLogic->OnMissionCompleted(10);

   //     _objects.erase(it);
   //     //return없었음.
   //     return true;
   // }
   // return true;
    std::cout << "[REMOVE] try id=" << objectId
        << " size(before)=" << _objects.size() << '\n';

    auto it = _objects.find(objectId);
    if (it == _objects.end())
    {
        std::cout << "[REMOVE] NOT FOUND id=" << objectId << '\n';
        return false;
    }

    std::cout << "[REMOVE] FOUND id=" << objectId
        << " modelType=" << it->second.modelType
        << " alive=" << it->second.alive
        << '\n';

    size_t erased = _objects.erase(objectId);
    DeleteWorldObjectFromDB(objectId);

    std::cout << "[REMOVE] erased=" << erased
        << " id=" << objectId
        << " size(after)=" << _objects.size() << '\n';

    return erased > 0;
}

void GlobalWorld::UpdateGrowth(uint64 nowMs, ESeason season, EWeather weather)
{
    constexpr float secondsToMature = 60.0f;           
    constexpr float baseRatePerSec = 100.0f / secondsToMature;
    constexpr uint64 kMinStepMs = 250;                

//const float growthMul = GWorldEnv.GetGrowthMultiplier();

    WRITE_LOCK;
    for (auto& [id, obj] : _objects)
    {
        if (!obj.alive)
            continue;

        if (!GWorldEnv.IsCropModel(obj.modelType))
            continue;

        if (obj.lastGrowMs == 0 )
        {
            obj.lastGrowMs = nowMs;
            std::cout << "[GROWTH] init id=" << id << " growth=0\n";
            continue;
        }

       /* uint64 dtMs = nowMs - obj.lastGrowMs;
        if (dtMs < kMinStepMs)
            continue;

        obj.lastGrowMs = nowMs;

        float dtSec = dtMs / 1000.0f;
        obj.growth += dtSec * baseRatePerSec;*/
        uint64 dtMs = nowMs - obj.lastGrowMs;
        if (dtMs < kMinStepMs)
            continue;

        obj.lastGrowMs = nowMs;

        float dtSec = static_cast<float>(dtMs) / 1000.0f;

        std::cout << "[BEFORE ADD] id=" << id
            << " growth=" << obj.growth
            << " lastGrowMs=" << obj.lastGrowMs
            << " dtMs=" << dtMs
            << " objModelType = " << obj.modelType
            << '\n';

        obj.growth += dtSec * baseRatePerSec;

        std::cout << "[A] after add id=" << id
            << " growth=" << obj.growth
            << " dtMs=" << dtMs
            << " objModelType = " << obj.modelType
            << '\n';

        Protocol::S_SEEDGROWTH pkt;
        if (70.0f> obj.growth &&obj.growth >= 25.0f && obj.modelType == 210) {
            obj.modelType = 200;
        }else if(100.0f>obj.growth >= 70.0f && obj.modelType == 200){
            obj.modelType = 201;
		}
		else if(125.0f> obj.growth &&  obj.growth >= 100.0f && obj.modelType == 201){
            obj.modelType = 202;
		}
		else if (obj.growth >= 125.0f && obj.modelType == 202) {
			obj.modelType = 203;
		}
		else if (70.0f > obj.growth >= 25.0f && obj.modelType == 310) {
			obj.modelType = 300;
		}
		else if (100.0f > obj.growth && obj.growth >= 70.0f && obj.modelType == 300) {
			obj.modelType = 301;
		}
		else if (125.0f > obj.growth && obj.growth >= 100.0f && obj.modelType == 301) {
			obj.modelType = 302;
		}
        else if (obj.growth >= 125.0f && obj.modelType == 302) {
            obj.modelType = 303;
        }
		else if (70.0f > obj.growth && obj.growth >= 25.0f && obj.modelType == 410) {
			obj.modelType = 400;
		}
		else if (100.0f > obj.growth && obj.growth >= 70.0f && obj.modelType == 400) {
			obj.modelType = 401;
		}
		else if (125.0f > obj.growth && obj.growth >= 100.0f && obj.modelType == 401) {
			obj.modelType = 402;
		}
        else if (obj.growth >= 125.0f && obj.modelType == 402) {
            obj.modelType = 403;
        }
		else if (70.0f > obj.growth && obj.growth >= 25.0f && obj.modelType == 510) {
			obj.modelType = 500;
		}
		else if (100.0f > obj.growth && obj.growth >= 70.0f && obj.modelType == 500) {
			obj.modelType = 501;
		}
		else if (125.0f > obj.growth && obj.growth >= 100.0f && obj.modelType == 501) {
			obj.modelType = 502;
		}
        else if (obj.growth >= 125.0f && obj.modelType == 502) {
            obj.modelType = 503;
        }
  //      pkt.set_growth(obj.growth);
  //      pkt.set_object_id(id);
		//auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		//GSessionManager.Broadcast(sendBuffer);
  //      std::cout
  //          << "[GROWTH] id=" << id
  //          << " model=" << obj.modelType
  //          << " -> " << obj.growth
  //          << " (mul="  << ")\n";
    
        pkt.set_growth(obj.growth);
        pkt.set_object_id(id);
        {
            DBWorldObjectData dbData;
            dbData.objectId = obj.id;
            dbData.modelType = obj.modelType;
            dbData.posX = obj.x;
            dbData.posY = obj.y;
            dbData.posZ = obj.z;
            dbData.rotX = obj.qx;
            dbData.rotY = obj.qy;
            dbData.rotZ = obj.qz;
            dbData.rotW = obj.qw;
            dbData.hp = obj.hp;
            dbData.growth = obj.growth;
            
            UpsertWorldObjectToDB(dbData);
        }

        std::cout << "[B] after pkt id=" << id
            << " growth=" << obj.growth
            << " objModelType = " << obj.modelType
            << '\n';

        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

        std::cout << "[B-2] after MakeSendBuffer id=" << id
            << " growth=" << obj.growth
            << " objModelType = " << obj.modelType
            << '\n';

        GSessionManager.Broadcast(sendBuffer);

        std::cout << "[C] after broadcast id=" << id
            << " growth=" << obj.growth
            << " objModelType = " << obj.modelType
            << '\n';
    }
}
uint32 GlobalWorld::StringToModelType(char* name)
{
    if (!strcmp(name, "Tree01"))   return 630;
    if (!strcmp(name, "Tree02"))   return 631;

    if (!strcmp(name, "Rock01"))   return 610;
    if (!strcmp(name, "Rock02"))   return 611;

    if (!strcmp(name, "Grass01"))  return 640;
    if (!strcmp(name, "Grass02"))  return 641;

    if (!strcmp(name, "Corn01"))   return 200;
    if (!strcmp(name, "Corn02"))   return 201;
    if (!strcmp(name, "Corn03"))   return 202;
    if (!strcmp(name, "Corn04"))   return 203;
    if (!strcmp(name, "CornSeed")) return 210;

    return 0;
}
void GlobalWorld::ParseMultiModelBlock(ifstream& InFile, const char* endTag, PacketSessionRef& session)
{
    char token[64];
    uint32 currentModelType = 0;

    while (InFile >> token)
    {
        if (!strcmp(token, endTag))
            break;

        if (!strcmp(token, "<Model>"))
        {
            char modelName[64];
            InFile >> modelName;
            currentModelType = StringToModelType(modelName);
        }
        else if (!strcmp(token, "<Positions>"))
        {
            while (InFile >> token)
            {
                if (!strcmp(token, "</Positions>"))
                    break;

                float x = static_cast<float>(atof(token));
                float z = 0.0f;
                InFile >> z;

                WorldObject obj;
                obj.id = GetSpawnIdGen();
                obj.modelType = currentModelType;
                obj.x = x;
                obj.z = z;
                obj.y = 0.0f;
                obj.alive = true;
                obj.init = true;

                _objects[obj.id] = obj;

                DBWorldObjectData dbData;
                dbData.objectId = obj.id;
                dbData.modelType = obj.modelType;
                dbData.posX = obj.x;
                dbData.posY = obj.y;
                dbData.posZ = obj.z;
                dbData.rotX = obj.qx;
                dbData.rotY = obj.qy;
                dbData.rotZ = obj.qz;
                dbData.rotW = obj.qw;
                dbData.hp = obj.hp;
                dbData.growth = obj.growth;

                UpsertWorldObjectToDB(dbData);
            }
        }
    }
    Protocol::S_WORLD_INIT pkt;
    for (auto obj : _objects)
    {
        auto it = obj.second;
        auto o = pkt.add_objects();
        o->mutable_transform()->set_px(it.x);
        o->mutable_transform()->set_py(it.y);
        o->mutable_transform()->set_pz(it.z);
        o->mutable_transform()->set_qx(it.qx);
        o->mutable_transform()->set_qy(it.qy);
        o->mutable_transform()->set_qz(it.qz);
        o->mutable_transform()->set_qw(it.qw);
        o->set_model_type(it.modelType);
        o->set_temp_id(it.id);
    }
    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    session->Send(sendBuffer);
}
void GlobalWorld::ParseSingleModelBlock(ifstream& InFile, const char* endTag, PacketSessionRef& session)
{
    char token[64];
    uint32 currentModelType = 0;
    while (InFile >> token)
    {
        if (!strcmp(token, endTag))
            break;

        if (!strcmp(token, "<Model>"))
        {
            char modelName[64];
            InFile >> modelName;
            currentModelType = StringToModelType(modelName);
        }
        else if (!strcmp(token, "</Model>"))
        {
            continue;
        }
        else if (!strcmp(token, "<Count>"))
        {
            int count;
            InFile >> count;
        }
        else if (!strcmp(token, "</Count>"))
        {
            continue;
        }
        else if (!strcmp(token, "<Positions>"))
        {
            while (InFile >> token)
            {
                if (!strcmp(token, "</Positions>"))
                    break;

                float x = static_cast<float>(atof(token));
                float z = 0.0f;
                InFile >> z;

                WorldObject obj;
                obj.id = GetSpawnIdGen();
                obj.modelType = currentModelType;
                obj.x = x;
                obj.z = z;
                obj.y = 0.0f;
                obj.alive = true;
                obj.init = true;

                _objects[obj.id] = obj;

                DBWorldObjectData dbData;
                dbData.objectId = obj.id;
                dbData.modelType = obj.modelType;
                dbData.posX = obj.x;
                dbData.posY = obj.y;
                dbData.posZ = obj.z;
                dbData.rotX = obj.qx;
                dbData.rotY = obj.qy;
                dbData.rotZ = obj.qz;
                dbData.rotW = obj.qw;
                dbData.hp = obj.hp;
                dbData.growth = obj.growth;

                UpsertWorldObjectToDB(dbData);

            }
        }
    }
    Protocol::S_WORLD_INIT pkt;
    for (auto obj : _objects)
    {
        auto it = obj.second;
        auto o = pkt.add_objects();
        o->mutable_transform()->set_px(it.x);
        o->mutable_transform()->set_py(it.y);
        o->mutable_transform()->set_pz(it.z);
        o->mutable_transform()->set_qx(it.qx);
        o->mutable_transform()->set_qy(it.qy);
        o->mutable_transform()->set_qz(it.qz);
        o->mutable_transform()->set_qw(it.qw);
        o->set_model_type(it.modelType);
        o->set_temp_id(it.id);
    }
    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
    session->Send(sendBuffer);
}
//void GlobalWorld::ReadTextFile(char* pstrFileName, PacketSessionRef& session)
//{
//    ifstream readFile(pstrFileName);
//    char token[64];
//    cout << "[ReadTextFile] open success: " << pstrFileName << endl;
//
//    while (readFile >> token)
//    {
//        if (!strcmp(token, "<Stage>"))
//        {
//            continue;
//        }
//        else if (!strcmp(token, "</Stage>"))
//        {
//            break;
//        }
//        else if (!strcmp(token, "<Index>"))
//        {
//        }
//        else if (!strcmp(token, "</Index>"))
//        {
//            continue;
//        }
//        else if (!strcmp(token, "<Rock>"))
//        {
//            ParseMultiModelBlock(readFile,  "</Rock>", session);
//        }
//        else if (!strcmp(token, "<Grass>"))
//        {
//            ParseMultiModelBlock(readFile, "</Grass>", session);
//        }
//        else if (!strcmp(token, "<Tree>"))
//        {
//            ParseSingleModelBlock(readFile,"</Tree>", session);
//        }
//    }
//
//    readFile.close();
//
//   // GWorld.ClSpawn()
//}
void GlobalWorld::ReadTextFile(char* pstrFileName, PacketSessionRef& session)
{
    if (!_initialized)
    {
        _initialized = true;
        ifstream readFile(pstrFileName);
        if (!readFile.is_open())
        {
            cout << "[ReadTextFile] open failed: " << pstrFileName << endl;
            return;
        }

        cout << "[ReadTextFile] open success: " << pstrFileName << endl;

        std::string token;

        while (readFile >> token)
        {
            // UTF-8 BOM 제거
            if (token.size() >= 3 &&
                (unsigned char)token[0] == 0xEF &&
                (unsigned char)token[1] == 0xBB &&
                (unsigned char)token[2] == 0xBF)
            {
                token.erase(0, 3);
            }

            if (token == "<Stage>")
            {
                continue;
            }
            else if (token == "</Stage>")
            {
                break;
            }
            else if (token == "<Index>")
            {
                std::string indexValue;
                readFile >> indexValue;
                continue;
            }
            else if (token == "</Index>")
            {
                continue;
            }
            else if (token == "<Rock>")
            {
                cout << "[ReadTextFile] ParseMultiModelBlock <Rock>" << endl;
                ParseMultiModelBlock(readFile, "</Rock>", session);
            }
            else if (token == "<Grass>")
            {
                cout << "[ReadTextFile] ParseMultiModelBlock <Grass>" << endl;
                ParseMultiModelBlock(readFile, "</Grass>", session);
            }
            else if (token == "<Tree>")
            {
                cout << "[ReadTextFile] ParseSingleModelBlock <Tree>" << endl;
                ParseSingleModelBlock(readFile, "</Tree>", session);
            }
            else
            {
                cout << "[ReadTextFile] unknown token: " << token << endl;
            }
        }

        readFile.close();
    }
}
bool GlobalWorld::ApplySeedGrowthRule(uint64 objectId, bool hasCeiling)
{
    float before = 0.0f;
    float after = 0.0f;
    uint32 modelType = 0;

    {
        WRITE_LOCK;

        auto it = _objects.find(objectId);
        if (it == _objects.end())
            return false;

        WorldObject& obj = it->second;
        if (!obj.alive)
            return false;
        if (!GWorldEnv.IsCropModel(obj.modelType))
            return false;

        modelType = obj.modelType;
        before = obj.growth;

         const EWeather w = GWorldEnv.GetWeather();
        const bool shouldDecrease = (!hasCeiling) || (w != EWeather::Clear);

        if (shouldDecrease)
        {
            obj.growth = Clamp01(obj.growth - 1.0f);
        }

        after = obj.growth;
    }

}

bool GlobalWorld::LoadFromDB()
{
    WRITE_LOCK;

    auto rows = LoadWorldObjectsFromDB();
    if (rows.empty())
        return false;

    _objects.clear();

    uint64 maxId = 0;

    for (const auto& row : rows)
    {
        WorldObject obj;
        obj.id = row.objectId;
        obj.modelType = row.modelType;
        obj.x = row.posX;
        obj.y = row.posY;
        obj.z = row.posZ;
        obj.qx = row.rotX;
        obj.qy = row.rotY;
        obj.qz = row.rotZ;
        obj.qw = row.rotW;
        obj.hp = row.hp;
        obj.growth = row.growth;
        obj.alive = true;
        obj.init = true;

        _objects[obj.id] = obj;

        if (obj.id > maxId)
            maxId = obj.id;
    }

    _idGen.store(maxId + 1);

    cout << "[GlobalWorld] loaded from DB. count=" << _objects.size() << endl;
    return true;
}