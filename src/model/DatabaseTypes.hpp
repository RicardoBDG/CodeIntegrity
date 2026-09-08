#ifndef DATABASE_TYPES_HPP
#define DATABASE_TYPES_HPP

#include <QString>
#include <QDateTime>

enum class UserRole
{
    Administrator,
    Teacher
};

inline QString roleToString(UserRole role)
{
    switch(role) {
    case UserRole::Administrator: return "Administrador";
    case UserRole::Teacher: return "Profesor";
    }
    return "";
}

inline UserRole stringToRole(const QString& roleStr)
{
    if (roleStr == "Administrador") return UserRole::Administrator;
    if (roleStr == "Profesor") return UserRole::Teacher;
    return UserRole::Teacher;
}

struct UserInfo
{
    int     id;
    QString nombre;
    QString apellido;
    QString email;
    QString contrasena;
    QString tipoRol;
};

struct SubjectInfo {
    int     id;
    QString nombre;
    QString descripcion;
    int     academicYearId = 0;
};

struct AcademicYearInfo
{
    int     id;
    QString año;
};

struct TaskInfo
{
    int       id;
    QString   titulo;
    QString   descripcion;
    QDateTime fechaLimite;
};

#endif
