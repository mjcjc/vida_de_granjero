#include "pch.h"
#include "WorldObjectDB.h"
#include "DBManager.h"
#include <mysql/jdbc.h>

void CreateWorldObjectTableIfNeeded()
{
    DBManager::Enqueue([]()
        {
            auto conn = DBManager::Pool().Acquire();
            if (conn == nullptr)
                return;

            try
            {
                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement(
                        "CREATE TABLE IF NOT EXISTS world_objects ("
                        "object_id BIGINT PRIMARY KEY,"
                        "model_type INT NOT NULL,"
                        "pos_x FLOAT NOT NULL DEFAULT 0,"
                        "pos_y FLOAT NOT NULL DEFAULT 0,"
                        "pos_z FLOAT NOT NULL DEFAULT 0,"
                        "rot_x FLOAT NOT NULL DEFAULT 0,"
                        "rot_y FLOAT NOT NULL DEFAULT 0,"
                        "rot_z FLOAT NOT NULL DEFAULT 0,"
                        "rot_w FLOAT NOT NULL DEFAULT 1,"
                        "hp INT NOT NULL DEFAULT 0,"
                        "growth INT NOT NULL DEFAULT 0,"
                        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP "
                        "ON UPDATE CURRENT_TIMESTAMP)"
                    )
                );

                pstmt->execute();
            }
            catch (const sql::SQLException& e)
            {
                cout << "[DB] CreateWorldObjectTableIfNeeded failed: "
                    << e.what() << endl;
            }

            DBManager::Pool().Release(std::move(conn));
        });
}

void UpsertWorldObjectToDB(const DBWorldObjectData& data)
{
    DBManager::Enqueue([data]()
        {
            auto conn = DBManager::Pool().Acquire();
            if (conn == nullptr)
                return;

            try
            {
                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement(
                        "INSERT INTO world_objects "
                        "(object_id, model_type, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_w, hp, growth) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                        "ON DUPLICATE KEY UPDATE "
                        "model_type = VALUES(model_type), "
                        "pos_x = VALUES(pos_x), "
                        "pos_y = VALUES(pos_y), "
                        "pos_z = VALUES(pos_z), "
                        "rot_x = VALUES(rot_x), "
                        "rot_y = VALUES(rot_y), "
                        "rot_z = VALUES(rot_z), "
                        "rot_w = VALUES(rot_w), "
                        "hp = VALUES(hp), "
                        "growth = VALUES(growth)"
                    )
                );

                pstmt->setUInt64(1, data.objectId);
                pstmt->setUInt(2, data.modelType);
                pstmt->setDouble(3, data.posX);
                pstmt->setDouble(4, data.posY);
                pstmt->setDouble(5, data.posZ);
                pstmt->setDouble(6, data.rotX);
                pstmt->setDouble(7, data.rotY);
                pstmt->setDouble(8, data.rotZ);
                pstmt->setDouble(9, data.rotW);
                pstmt->setUInt(10, data.hp);
                pstmt->setUInt(11, data.growth);

                pstmt->execute();
            }
            catch (const sql::SQLException& e)
            {
                cout << "[DB] UpsertWorldObjectToDB failed: "
                    << e.what() << endl;
            }

            DBManager::Pool().Release(std::move(conn));
        });
}

void DeleteWorldObjectFromDB(uint64 objectId)
{
    DBManager::Enqueue([objectId]()
        {
            auto conn = DBManager::Pool().Acquire();
            if (conn == nullptr)
                return;

            try
            {
                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement(
                        "DELETE FROM world_objects WHERE object_id = ?"
                    )
                );

                pstmt->setUInt64(1, objectId);
                pstmt->execute();
            }
            catch (const sql::SQLException& e)
            {
                cout << "[DB] DeleteWorldObjectFromDB failed: "
                    << e.what() << endl;
            }

            DBManager::Pool().Release(std::move(conn));
        });
}

bool IsWorldObjectTableEmpty()
{
    auto conn = DBManager::Pool().Acquire();
    if (conn == nullptr)
        return true;

    bool isEmpty = true;

    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT COUNT(*) AS cnt FROM world_objects")
        );

        std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
        if (rs->next())
        {
            isEmpty = (rs->getUInt64("cnt") == 0);
        }
    }
    catch (const sql::SQLException& e)
    {
        cout << "[DB] IsWorldObjectTableEmpty failed: " << e.what() << endl;
    }

    DBManager::Pool().Release(std::move(conn));
    return isEmpty;
}

std::vector<DBWorldObjectData> LoadWorldObjectsFromDB()
{
    std::vector<DBWorldObjectData> out;

    auto conn = DBManager::Pool().Acquire();
    if (conn == nullptr)
        return out;

    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT object_id, model_type, pos_x, pos_y, pos_z, "
                "rot_x, rot_y, rot_z, rot_w, hp, growth "
                "FROM world_objects"
            )
        );

        std::unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());

        while (rs->next())
        {
            DBWorldObjectData row;
            row.objectId = rs->getUInt64("object_id");
            row.modelType = rs->getUInt("model_type");
            row.posX = rs->getDouble("pos_x");
            row.posY = rs->getDouble("pos_y");
            row.posZ = rs->getDouble("pos_z");
            row.rotX = rs->getDouble("rot_x");
            row.rotY = rs->getDouble("rot_y");
            row.rotZ = rs->getDouble("rot_z");
            row.rotW = rs->getDouble("rot_w");
            row.hp = rs->getUInt("hp");
            row.growth = static_cast<float>(rs->getDouble("growth"));
            out.push_back(row);
        }
    }
    catch (const sql::SQLException& e)
    {
        cout << "[DB] LoadWorldObjectsFromDB failed: " << e.what() << endl;
    }

    DBManager::Pool().Release(std::move(conn));
    return out;
}