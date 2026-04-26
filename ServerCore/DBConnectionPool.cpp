#include "pch.h"
#include "DBConfig.h"
#include "DBConnectionPool.h"

bool DBConnectionPool::Init(const DBConfig& cfg)
{
    LockGuard lock(_mtx);

    _running.store(true);

    try
    {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        while (!_pool.empty())
            _pool.pop();

        for (int32 i = 0; i < cfg.PoolSize(); i++)
        {
            std::unique_ptr<sql::Connection> con(driver->connect(cfg.Host(), cfg.User(), cfg.Pass()));
            con->setSchema(cfg.Schema());
            _pool.push(std::move(con));
        }

        cout << "[DB] Pool Init OK. size=" << cfg.PoolSize() << endl;
        return true;
    }
    catch (const sql::SQLException& e)
    {
        cout << "[DB] Pool Init FAILED: " << e.what()
            << " (ErrorCode=" << e.getErrorCode()
            << ", SQLState=" << e.getSQLState() << ")" << endl;

        _running.store(false);
        return false;
    }
}

void DBConnectionPool::Shutdown()
{
    {
        LockGuard lock(_mtx);
        _running.store(false);

        while (!_pool.empty())
            _pool.pop();
    }
    _cv.notify_all();
}

std::unique_ptr<sql::Connection> DBConnectionPool::Acquire()
{
    UniqueLock lock(_mtx);

    _cv.wait(lock, [&]()
        {
            return (_running.load() == false) || (_pool.empty() == false);
        });

    if (_running.load() == false)
        return nullptr;

    auto conn = std::move(_pool.front());
    _pool.pop();
    return conn;
}

void DBConnectionPool::Release(std::unique_ptr<sql::Connection> conn)
{
    if (conn == nullptr)
        return;

    {
        LockGuard lock(_mtx);
        if (_running.load() == false)
            return;

        _pool.push(std::move(conn));
    }

    _cv.notify_one();
}

int32 DBConnectionPool::GetFreeCount() const
{
    LockGuard lock(_mtx);
    return static_cast<int32>(_pool.size());
}