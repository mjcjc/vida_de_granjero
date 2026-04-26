#pragma once

struct DBWorldObjectData
{
    uint64 objectId = 0;
    uint32 modelType = 0;

    float posX = 0.f;
    float posY = 0.f;
    float posZ = 0.f;

    float rotX = 0.f;
    float rotY = 0.f;
    float rotZ = 0.f;
    float rotW = 1.f;

    uint32 hp = 0;
    uint32 growth = 0;
};

void CreateWorldObjectTableIfNeeded();
void UpsertWorldObjectToDB(const DBWorldObjectData& data);
void DeleteWorldObjectFromDB(uint64 objectId);

bool IsWorldObjectTableEmpty();
std::vector<DBWorldObjectData> LoadWorldObjectsFromDB();