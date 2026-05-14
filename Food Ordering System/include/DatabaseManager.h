#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#undef byte
#include <mysql.h>
#include <string>
#include <stdexcept>

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& msg) : std::runtime_error(msg) {}
};

class DatabaseManager {
private:
    MYSQL* connection;
    static DatabaseManager* instance;
    std::string host, user, password, dbName;
    int port;
    DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
public:
    static DatabaseManager* getInstance();
    ~DatabaseManager();
    bool connect(const std::string& h, const std::string& u,
                 const std::string& p, const std::string& db, int port = 3306);
    void disconnect();
    bool isConnected() const;
    MYSQL_RES* executeQuery(const std::string& query);
    bool executeUpdate(const std::string& query);
    int getLastInsertId();
    std::string escapeString(const std::string& str);
    bool initializeDatabase();
};

#endif
