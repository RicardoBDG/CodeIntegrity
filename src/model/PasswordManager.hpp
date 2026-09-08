#ifndef PASSWORD_MANAGER_HPP
#define PASSWORD_MANAGER_HPP

#include <QString>

class PasswordManager
{
public:
    static QString hashPassword(const QString& password);
    static bool verifyPassword(const QString& password, const QString& hash);

private:
    static QString generateSalt();
};

#endif
