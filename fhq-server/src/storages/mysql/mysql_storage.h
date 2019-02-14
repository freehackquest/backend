#ifndef MYSQL_STORAGE_H
#define MYSQL_STORAGE_H

#include <storages.h>
#include <map>
#include <mysql/mysql.h>

class MySqlStorageConnection : public StorageConnection {
    public:
        MySqlStorageConnection(MYSQL *pConn);
        ~MySqlStorageConnection();
        virtual bool executeQuery(const std::string &sQuery);
        virtual std::string lastDatabaseVersion();
        virtual bool insertUpdateInfo(const std::string &sVersion, const std::string &sDescription);
    private:
        MYSQL *m_pConnection;
        std::string escapeString(const std::string &sValue);
};

class MySqlStorage : public Storage {
    public:
        MySqlStorage();
        static std::string type() { return "mysql"; };
        virtual bool applyConfigFromFile(const std::string &sFilePath);
        virtual StorageConnection *connect();
        virtual void clean();
        virtual std::vector<std::string> prepareSqlQueries(StorageStruct &storageStruct);

    private:
        std::string generateLineColumnForSql(StorageStructColumn &c);
        std::string TAG;
        std::string m_sDatabaseHost;
        std::string m_sDatabaseName;
        std::string m_sDatabaseUser;
        std::string m_sDatabasePass;
        int m_nDatabasePort;
};

#endif // MYSQL_STORAGE_H
