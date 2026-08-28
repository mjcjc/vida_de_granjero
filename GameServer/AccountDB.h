#pragma once

enum class AccountLoginResult {
    Success,
    Created,
    WrongPassword,
    InvalidInput,
    DatabaseError
};

struct AccountAuthResult {
    AccountLoginResult result = AccountLoginResult::DatabaseError;
    uint64 accountDbId = 0;
};

bool CreateAccountTableIfNeeded();
AccountAuthResult AuthenticateOrCreateAccount(const std::string& accountId, const std::string& password);
