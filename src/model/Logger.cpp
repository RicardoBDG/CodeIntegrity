#include "Logger.hpp"
#include <QDateTime>
#include <iostream>

void Logger::log(LogLevel level, const QString& message)
{
    QString timestamp = getTimestamp();
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    std::cout << logMessage.toStdString() << std::endl;
}

void Logger::logQuery(const QString& query, const QSqlError& error)
{
    QString message = QString("Query Error: %1\nQuery: %2").arg(error.text(), query);
    log(ERROR_LEVEL, message);
}

QString Logger::levelToString(LogLevel level)
{
    switch(level)
    {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR_LEVEL: return "ERROR";
        default: return "UNKNOWN";
    }
}

QString Logger::getTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}
