#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <QtSql>
#include "DatabaseTypes.hpp"
#include <vector>

class DataBaseManager
{
public:
    static DataBaseManager* getInstance();
    ~DataBaseManager();

    bool connectToDatabase();
    bool createDataBaseTables();
    void closeConnectionDB();

    QString authenticateUser(const QString& email, const QString& password);
    int     getCurrentUserId(const QString& email);

    std::vector<UserInfo>         getUsers();
    std::vector<SubjectInfo>      getSubjects();
    std::vector<AcademicYearInfo> getAcademicYears();
    std::vector<SubjectInfo>      getSubjectsByTeacher(int userId);
    std::vector<TaskInfo>         getTasksBySubject(int subjectId);
    std::vector<SubjectInfo>      getSubjectsByAcademicYear(int academicYearId);
    bool getUserByEmail(const QString& email, UserInfo& usuario, QString* errorMsg = nullptr);

    bool insertUser(const QString& nombre, const QString& apellido,
                    const QString& email, const QString& contrasena,
                    const QString& tipoRol, QString* errorMsg = nullptr);

    bool insertSubject(const QString& nombre, const QString& descripcion,
                       int academicYearId, QString* errorMsg = nullptr);

    bool insertAcademicYear(const QString& year, QString* errorMsg = nullptr);

    bool insertTask(const QString& titulo, const QString& descripcion,
                    int subjectId, int academicYearId,
                    const QDateTime& fechaLimite, QString* errorMsg = nullptr);

    bool updateUser(int id, const QString& nombre, const QString& apellido,
                    const QString& email, const QString& contrasena,
                    const QString& tipoRol, QString* errorMsg = nullptr);

    // Igual que updateUser, pero sin tocar la columna Contraseña. Se usa cuando
    // el admin deja el campo de contraseña en blanco para no cambiarla: pasar el
    // valor ya hasheado de vuelta a updateUser() haría PasswordManager::hashPassword
    // sobre un hash ya existente y dejaría al usuario sin poder volver a iniciar sesión.
    bool updateUserKeepingPassword(int id, const QString& nombre, const QString& apellido,
                                   const QString& email, const QString& tipoRol,
                                   QString* errorMsg = nullptr);

    bool updateSubject(int id, const QString& nombre, const QString& descripcion,
                       QString* errorMsg = nullptr);

    bool updateAcademicYear(int id, const QString& year, QString* errorMsg = nullptr);

    bool updateTask(int id, const QString& titulo, const QString& descripcion,
                    const QDateTime& fechaLimite, QString* errorMsg = nullptr);

    bool deleteUser(int id, QString* errorMsg = nullptr);
    bool deleteSubject(int id, QString* errorMsg = nullptr);
    bool deleteAcademicYear(int id, QString* errorMsg = nullptr);
    bool deleteTask(int id, QString* errorMsg = nullptr);

    bool assignTeacherToSubject(int userId, int subjectId, int academicYearId, QString* errorMsg = nullptr);
    bool removeTeacherFromSubject(int userId, int subjectId, int academicYearId, QString* errorMsg = nullptr);
    std::vector<UserInfo> getTeachersBySubject(int subjectId, int academicYearId);

private:
    static DataBaseManager* instance;
    QSqlDatabase mDataBase;

    DataBaseManager();

    bool executeQuery(QSqlQuery& query, const QString& errorMsg = "");
    bool updateRecord(const QString& table, const QString& setClause,
                      const QString& whereClause, const QMap<QString, QVariant>& bindings);
    bool deleteRecord(const QString& table, int id);

    void createDefaultRoles();
    void createDefaultUsers();
    bool userExists(const QString& email);
};

#endif
