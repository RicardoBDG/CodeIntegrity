#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <QString>
#include <QSqlError>

class Logger
{
    public:

        enum LogLevel
        {
            DEBUG,
            INFO,
            WARNING,
            ERROR_LEVEL
        };

        static void log(LogLevel level, const QString& message);
        static void logQuery(const QString& query, const QSqlError& error);

    private:
        static QString levelToString(LogLevel level);
        static QString getTimestamp();
};

#endif
