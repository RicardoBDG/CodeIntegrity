#include "PasswordManager.hpp"
#include <QCryptographicHash>
#include <QUuid>
#include <QStringList>

QString PasswordManager::hashPassword(const QString& password)
{
    QString salt = generateSalt();

    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(
        combined.toUtf8(),
        QCryptographicHash::Sha256
        );

    return salt + ":" + QString::fromUtf8(hash.toHex());
}

bool PasswordManager::verifyPassword(const QString& password, const QString& hash)
{
    QStringList parts = hash.split(":");
    if (parts.size() != 2)
    {
        return false;
    }

    QString salt = parts[0];
    QString storedHash = parts[1];

    QString combined = password + salt;
    QByteArray computedHash = QCryptographicHash::hash(
        combined.toUtf8(),
        QCryptographicHash::Sha256
        );

    return storedHash == QString::fromUtf8(computedHash.toHex());
}

QString PasswordManager::generateSalt()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}
