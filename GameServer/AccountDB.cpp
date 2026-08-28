#include "pch.h"
#include "AccountDB.h"
#include "DBManager.h"
#include <mysql/jdbc.h>

bool IsValidAccountInput(const std::string& accountId, const std::string& password) {
    if (accountId.size() < 3 || accountId.size() > 32 || password.size() < 4 || password.size() > 64) {
        return false;
    }

    for (const unsigned char value : accountId) {
        if (!(std::isalnum(value) || value == '_' || value == '-')) {
            return false;
        }
    }

    return true;
}

bool CreateAccountTableIfNeeded() {
    auto conn = DBManager::Pool().Acquire();
    if (conn == nullptr) {
        return false;
    }

    bool success = false;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("CREATE TABLE IF NOT EXISTS accounts (account_db_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, account_id VARCHAR(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL UNIQUE, password_hash VARCHAR(255) NOT NULL, created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, last_login_at TIMESTAMP NULL DEFAULT NULL)"));
        pstmt->execute();
        success = true;
    } catch (const sql::SQLException& e) {
        cout << "[DB] CreateAccountTableIfNeeded failed: " << e.what() << endl;
    }

    DBManager::Pool().Release(std::move(conn));
    return success;
}

AccountAuthResult AuthenticateOrCreateAccount(const std::string& accountId, const std::string& password) {
    if (!IsValidAccountInput(accountId, password) || !CreateAccountTableIfNeeded()) {
        return { AccountLoginResult::InvalidInput, 0 };
    }

    auto conn = DBManager::Pool().Acquire();
    if (conn == nullptr) {
        return { AccountLoginResult::DatabaseError, 0 };
    }

    AccountAuthResult authResult;
    try {
        std::unique_ptr<sql::PreparedStatement> select(conn->prepareStatement("SELECT account_db_id, password_hash FROM accounts WHERE account_id = ?"));
        select->setString(1, accountId);
        std::unique_ptr<sql::ResultSet> rows(select->executeQuery());

        if (rows->next()) {
            const uint64 accountDbId = rows->getUInt64("account_db_id");
            const std::string storedPassword = rows->getString("password_hash").asStdString();
            if (password != storedPassword) {
                authResult = { AccountLoginResult::WrongPassword, 0 };
            } else {
                std::unique_ptr<sql::PreparedStatement> update(conn->prepareStatement("UPDATE accounts SET last_login_at = CURRENT_TIMESTAMP WHERE account_db_id = ?"));
                update->setUInt64(1, accountDbId);
                update->execute();
                authResult = { AccountLoginResult::Success, accountDbId };
            }
        } else {
            std::unique_ptr<sql::PreparedStatement> insert(conn->prepareStatement("INSERT INTO accounts (account_id, password_hash, last_login_at) VALUES (?, ?, CURRENT_TIMESTAMP)"));
            insert->setString(1, accountId);
            insert->setString(2, password);
            insert->execute();
            authResult = { AccountLoginResult::Created, 0 };

            std::unique_ptr<sql::PreparedStatement> findCreated(conn->prepareStatement("SELECT account_db_id FROM accounts WHERE account_id = ?"));
            findCreated->setString(1, accountId);
            std::unique_ptr<sql::ResultSet> createdRows(findCreated->executeQuery());
            if (createdRows->next()) {
                authResult.accountDbId = createdRows->getUInt64("account_db_id");
            }
        }
    } catch (const sql::SQLException& e) {
        cout << "[DB] AuthenticateOrCreateAccount failed: " << e.what() << endl;
        authResult = { AccountLoginResult::DatabaseError, 0 };
    }

    DBManager::Pool().Release(std::move(conn));
    return authResult;
}
