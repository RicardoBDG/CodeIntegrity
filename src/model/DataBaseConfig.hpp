#ifndef DATABASE_CONFIG_HPP
#define DATABASE_CONFIG_HPP

#include <QString>

class DatabaseConfig
{
public:
    static constexpr const char* DEFAULT_HOST = "localhost";
    static constexpr const char* DATABASE = "postgres";
    static constexpr const char* DEFAULT_PORT = "5432";
    static constexpr const char* DEFAULT_USER = "postgres";
    static constexpr const char* DEFAULT_PASSWORD = "postgres";

    static QString getHost()
    {
        const QByteArray envHost = qgetenv("DB_HOST");
        return envHost.isEmpty() ? QString(DEFAULT_HOST) : QString::fromUtf8(envHost);
    }

    static QString getPort()
    {
        const QByteArray envPort = qgetenv("DB_PORT");
        return envPort.isEmpty() ? QString(DEFAULT_PORT) : QString::fromUtf8(envPort);
    }

    static QString getUsername()
    {
        const QByteArray envUser = qgetenv("DB_USER");
        return envUser.isEmpty() ? QString(DEFAULT_USER) : QString::fromUtf8(envUser);
    }

    static QString getPassword()
    {
        const QByteArray envPass = qgetenv("DB_PASSWORD");
        return envPass.isEmpty() ? QString(DEFAULT_PASSWORD) : QString::fromUtf8(envPass);
    }
};

#endif
