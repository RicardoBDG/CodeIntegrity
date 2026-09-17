#include "DataBaseManager.hpp"
#include "DatabaseConfig.hpp"
#include "PasswordManager.hpp"
#include "Logger.hpp"
#include <QRegularExpression>

DataBaseManager* DataBaseManager::instance = nullptr;

namespace {
// Regex simple pero mucho más estricta que el antiguo email.contains("@"):
// exige usuario, arroba, dominio con al menos un punto y TLD de 2+ letras.
// No pretende validar RFC 5322 al completo (nadie lo necesita aquí), solo
// filtrar los casos obviamente inválidos que "contains('@')" dejaba pasar
// (p.ej. "@", "a@b", "sin-arroba.com").
const QRegularExpression kEmailRegex(
    R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");

// Validaciones comunes a nombre/apellido/email/rol, usadas tanto al crear
// como al editar un usuario. La contraseña se valida aparte (ver
// validateUserFields) porque al editar sin cambiarla no hay contraseña que
// comprobar.
bool validateCommonUserFields(const QString& nombre, const QString& apellido,
                              const QString& email, const QString& tipoRol,
                              QString* errorMsg)
{
    if (nombre.trimmed().isEmpty() || apellido.trimmed().isEmpty() ||
        email.trimmed().isEmpty()  || tipoRol.trimmed().isEmpty())
    {
        if (errorMsg) *errorMsg = "Todos los campos son requeridos";
        return false;
    }
    if (!kEmailRegex.match(email).hasMatch())
    {
        if (errorMsg) *errorMsg = "Email inválido";
        return false;
    }
    if (tipoRol != "Administrador" && tipoRol != "Profesor")
    {
        if (errorMsg) *errorMsg = "Tipo de rol inválido";
        return false;
    }
    return true;
}

bool validateUserFields(const QString& nombre, const QString& apellido,
                        const QString& email, const QString& contrasena,
                        const QString& tipoRol, QString* errorMsg)
{
    if (!validateCommonUserFields(nombre, apellido, email, tipoRol, errorMsg))
        return false;

    if (contrasena.isEmpty())
    {
        if (errorMsg) *errorMsg = "Todos los campos son requeridos";
        return false;
    }
    if (contrasena.length() < 8)
    {
        if (errorMsg) *errorMsg = "Contraseña debe tener mínimo 8 caracteres";
        return false;
    }
    return true;
}
} // namespace

DataBaseManager::DataBaseManager() {}

DataBaseManager::~DataBaseManager()
{
    closeConnectionDB();
}

DataBaseManager* DataBaseManager::getInstance()
{
    if (instance == nullptr)
        instance = new DataBaseManager();
    return instance;
}

void DataBaseManager::closeConnectionDB()
{
    if (mDataBase.isOpen())
        mDataBase.close();
}

bool DataBaseManager::executeQuery(QSqlQuery& query, const QString& errorMsg)
{
    if (!query.exec())
    {
        Logger::logQuery(query.executedQuery(), query.lastError());
        if (!errorMsg.isEmpty())
            Logger::log(Logger::ERROR_LEVEL, errorMsg + ": " + query.lastError().text());
        return false;
    }
    return true;
}

bool DataBaseManager::updateRecord(const QString& table, const QString& setClause,
                                   const QString& whereClause, const QMap<QString, QVariant>& bindings)
{
    QSqlQuery query(mDataBase);
    query.prepare(QString("UPDATE %1 SET %2 WHERE %3").arg(table, setClause, whereClause));
    for (auto it = bindings.begin(); it != bindings.end(); ++it)
        query.bindValue(it.key(), it.value());
    return executeQuery(query, QString("Error actualizando %1").arg(table));
}

bool DataBaseManager::deleteRecord(const QString& table, int id)
{
    QSqlQuery query(mDataBase);

    // Todas las tablas de este esquema siguen el patrón "ID_<NombreTabla>"
    // (ID_Usuario, ID_Asignatura, ID_CursoAcademico, ID_Tarea), así que el
    // antiguo if/else por tabla era código muerto: nunca entraba en una
    // rama distinta del "else" genérico que ya cubría todos los casos.
    const QString idColumn = QString("ID_%1").arg(table);

    query.prepare(QString("DELETE FROM %1 WHERE %2 = :id").arg(table, idColumn));
    query.bindValue(":id", id);
    return executeQuery(query, QString("Error eliminando de %1").arg(table));
}

bool DataBaseManager::connectToDatabase()
{
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        mDataBase = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        mDataBase = QSqlDatabase::addDatabase("QPSQL");
        mDataBase.setHostName(DatabaseConfig::getHost());
        mDataBase.setDatabaseName(DatabaseConfig::DATABASE);
        mDataBase.setUserName(DatabaseConfig::getUsername());
        mDataBase.setPassword(DatabaseConfig::getPassword());
        mDataBase.setPort(DatabaseConfig::getPort().toInt());
    }

    if (!mDataBase.open())
    {
        Logger::log(Logger::ERROR_LEVEL, "Error de conexión: " + mDataBase.lastError().text());
        return false;
    }

    QSqlQuery query(mDataBase);
    query.exec("SET client_min_messages TO WARNING;");

    Logger::log(Logger::INFO, "Conectado a base de datos exitosamente");
    return true;
}

bool DataBaseManager::createDataBaseTables()
{
    const QStringList tables =
        {
            // Rol
            "CREATE TABLE IF NOT EXISTS Rol ("
            "Tipo_rol VARCHAR(20) PRIMARY KEY "
            "CHECK (Tipo_rol IN ('Administrador', 'Profesor')));",

            // Usuario
            "CREATE TABLE IF NOT EXISTS Usuario ("
            "ID_Usuario   SERIAL PRIMARY KEY, "
            "Tipo_rol     VARCHAR(20) NOT NULL, "
            "Nombre       VARCHAR(50) NOT NULL, "
            "Apellido     VARCHAR(50) NOT NULL, "
            "Email        VARCHAR(100) NOT NULL UNIQUE, "
            "Contraseña   VARCHAR(255) NOT NULL, "
            "FOREIGN KEY (Tipo_rol) REFERENCES Rol(Tipo_rol) "
            "ON DELETE RESTRICT ON UPDATE CASCADE);",

            // CursoAcademico
            "CREATE TABLE IF NOT EXISTS CursoAcademico ("
            "ID_CursoAcademico SERIAL PRIMARY KEY, "
            "Año VARCHAR(5) UNIQUE);",

            // Asignatura
            "CREATE TABLE IF NOT EXISTS Asignatura ("
            "ID_Asignatura     SERIAL PRIMARY KEY, "
            "ID_CursoAcademico INT NOT NULL, "
            "Nombre            VARCHAR(100) NOT NULL, "
            "Descripción       TEXT, "
            "FOREIGN KEY (ID_CursoAcademico) REFERENCES CursoAcademico(ID_CursoAcademico) "
            "ON DELETE CASCADE ON UPDATE CASCADE);",

            // Tarea
            "CREATE TABLE IF NOT EXISTS Tarea ("
            "ID_Tarea          SERIAL PRIMARY KEY, "
            "ID_Asignatura     INT NOT NULL, "
            "ID_CursoAcademico INT NOT NULL, "
            "Titulo            VARCHAR(150) NOT NULL, "
            "Descripcion       TEXT, "
            "FechaLimite       TIMESTAMP, "
            "FOREIGN KEY (ID_Asignatura) REFERENCES Asignatura(ID_Asignatura) "
            "ON DELETE CASCADE ON UPDATE CASCADE, "
            "FOREIGN KEY (ID_CursoAcademico) REFERENCES CursoAcademico(ID_CursoAcademico) "
            "ON DELETE CASCADE ON UPDATE CASCADE);",

            // Asignacion_Usuario_Asignatura
            "CREATE TABLE IF NOT EXISTS Asignacion_Usuario_Asignatura ("
            "ID_Asignacion     SERIAL PRIMARY KEY, "
            "ID_Usuario        INT NOT NULL, "
            "ID_Asignatura     INT NOT NULL, "
            "ID_CursoAcademico INT NOT NULL, "
            "CONSTRAINT asignacion_unica UNIQUE (ID_Usuario, ID_Asignatura, ID_CursoAcademico), "
            "FOREIGN KEY (ID_Usuario) REFERENCES Usuario(ID_Usuario) "
            "ON DELETE CASCADE ON UPDATE CASCADE, "
            "FOREIGN KEY (ID_Asignatura) REFERENCES Asignatura(ID_Asignatura) "
            "ON DELETE CASCADE ON UPDATE CASCADE, "
            "FOREIGN KEY (ID_CursoAcademico) REFERENCES CursoAcademico(ID_CursoAcademico) "
            "ON DELETE CASCADE ON UPDATE CASCADE);"
        };

    QSqlQuery query(mDataBase);
    for (const auto& table : tables)
    {
        if (!query.exec(table))
        {
            Logger::log(Logger::ERROR_LEVEL, "Error creando tabla: " + query.lastError().text());
            return false;
        }
    }

    Logger::log(Logger::INFO, "Tablas creadas/verificadas exitosamente");

    createDefaultRoles();
    createDefaultUsers();

    return true;
}

void DataBaseManager::createDefaultUsers()
{
    QString errorMsg;

    if (!userExists("alejandro.dominguez1@profesor.edu"))
    {
        bool ok = insertUser("Alejandro", "Domínguez",
                             "alejandro.dominguez1@profesor.edu",
                             "profesor123", "Profesor", &errorMsg);
        Logger::log(ok ? Logger::INFO : Logger::ERROR_LEVEL,
                    ok ? "Usuario Profesor creado automáticamente"
                       : "Error creando Profesor: " + errorMsg);
    }

    if (!userExists("carmen.molina@admin.edu"))
    {
        bool ok = insertUser("Carmen", "Molina",
                             "carmen.molina@admin.edu",
                             "admin123456", "Administrador", &errorMsg);
        Logger::log(ok ? Logger::INFO : Logger::ERROR_LEVEL,
                    ok ? "Usuario Admin creado automáticamente"
                       : "Error creando Admin: " + errorMsg);
    }
}

void DataBaseManager::createDefaultRoles()
{
    QSqlQuery query(mDataBase);

    query.exec("INSERT INTO Rol (Tipo_rol) VALUES ('Administrador') ON CONFLICT DO NOTHING;");
    query.exec("INSERT INTO Rol (Tipo_rol) VALUES ('Profesor')      ON CONFLICT DO NOTHING;");

    Logger::log(Logger::INFO, "Roles verificados/creados correctamente");
}

bool DataBaseManager::userExists(const QString& email)
{
    if (!mDataBase.isOpen()) return false;

    QSqlQuery query(mDataBase);
    query.prepare("SELECT COUNT(*) FROM Usuario WHERE Email = :email");
    query.bindValue(":email", email);

    if (executeQuery(query) && query.next())
        return query.value(0).toInt() > 0;
    return false;
}

QString DataBaseManager::authenticateUser(const QString& email, const QString& password)
{
    if (email.isEmpty() || password.isEmpty())
    {
        Logger::log(Logger::WARNING, "Intento de autenticación con campos vacíos");
        return "error";
    }

    if (!mDataBase.isOpen() && !connectToDatabase())
    {
        Logger::log(Logger::ERROR_LEVEL, "Base de datos no disponible para autenticación");
        return "error";
    }

    QSqlQuery query(mDataBase);
    query.prepare("SELECT Tipo_rol, Contraseña FROM Usuario WHERE Email = :email");
    query.bindValue(":email", email);

    if (executeQuery(query) && query.next())
    {
        QString tipoRol    = query.value(0).toString();
        QString storedHash = query.value(1).toString();

        if (PasswordManager::verifyPassword(password, storedHash))
        {
            Logger::log(Logger::INFO, QString("Usuario autenticado: %1").arg(email));
            return tipoRol;
        }
        Logger::log(Logger::WARNING, QString("Contraseña incorrecta para: %1").arg(email));
    }
    else
    {
        Logger::log(Logger::WARNING, QString("Usuario no encontrado: %1").arg(email));
    }

    return "error";
}

int DataBaseManager::getCurrentUserId(const QString& email)
{
    QSqlQuery query(mDataBase);
    query.prepare("SELECT ID_Usuario FROM Usuario WHERE Email = :email");
    query.bindValue(":email", email);

    if (executeQuery(query) && query.next())
        return query.value(0).toInt();

    Logger::log(Logger::WARNING, QString("No se encontró ID para: %1").arg(email));
    return -1;
}

std::vector<UserInfo> DataBaseManager::getUsers()
{
    std::vector<UserInfo> usuarios;
    QSqlQuery query(mDataBase);

    if (query.exec("SELECT ID_Usuario, Nombre, Apellido, Email, Contraseña, Tipo_rol FROM Usuario"))
    {
        while (query.next())
        {
            usuarios.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toString(),
                query.value(4).toString(),
                query.value(5).toString()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL, "Error obteniendo usuarios");
    }

    return usuarios;
}

std::vector<SubjectInfo> DataBaseManager::getSubjects()
{
    std::vector<SubjectInfo> asignaturas;
    QSqlQuery query(mDataBase);

    if (query.exec("SELECT ID_Asignatura, Nombre, Descripción, ID_CursoAcademico FROM Asignatura ORDER BY Nombre"))
    {
        while (query.next())
        {
            asignaturas.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toInt()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL, "Error obteniendo asignaturas");
    }

    return asignaturas;
}

std::vector<AcademicYearInfo> DataBaseManager::getAcademicYears()
{
    std::vector<AcademicYearInfo> cursos;
    QSqlQuery query(mDataBase);

    if (query.exec("SELECT ID_CursoAcademico, Año FROM CursoAcademico"))
    {
        while (query.next())
        {
            cursos.push_back({
                query.value(0).toInt(),
                query.value(1).toString()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL, "Error obteniendo cursos académicos");
    }

    return cursos;
}

std::vector<SubjectInfo> DataBaseManager::getSubjectsByTeacher(int userId)
{
    std::vector<SubjectInfo> asignaturas;
    QSqlQuery query(mDataBase);

    query.prepare("SELECT a.ID_Asignatura, a.Nombre, a.Descripción, a.ID_CursoAcademico "
                  "FROM Asignatura a "
                  "INNER JOIN Asignacion_Usuario_Asignatura aua ON a.ID_Asignatura = aua.ID_Asignatura "
                  "WHERE aua.ID_Usuario = :id_usuario "
                  "ORDER BY a.Nombre");
    query.bindValue(":id_usuario", userId);

    if (query.exec())
    {
        while (query.next())
        {
            asignaturas.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toInt()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL,
                    QString("Error obteniendo asignaturas del profesor %1").arg(userId));
    }

    return asignaturas;
}

std::vector<TaskInfo> DataBaseManager::getTasksBySubject(int subjectId)
{
    std::vector<TaskInfo> tareas;
    QSqlQuery query(mDataBase);

    query.prepare("SELECT ID_Tarea, Titulo, Descripcion, FechaLimite "
                  "FROM Tarea "
                  "WHERE ID_Asignatura = :id_asignatura "
                  "ORDER BY Titulo");
    query.bindValue(":id_asignatura", subjectId);

    if (query.exec())
    {
        while (query.next())
        {
            tareas.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toDateTime()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL,
                    QString("Error obteniendo tareas de la asignatura %1").arg(subjectId));
    }

    return tareas;
}

std::vector<SubjectInfo> DataBaseManager::getSubjectsByAcademicYear(int academicYearId)
{
    std::vector<SubjectInfo> asignaturas;
    QSqlQuery query(mDataBase);

    query.prepare("SELECT ID_Asignatura, Nombre, Descripción, ID_CursoAcademico "
                  "FROM Asignatura "
                  "WHERE ID_CursoAcademico = :id "
                  "ORDER BY Nombre");
    query.bindValue(":id", academicYearId);

    if (query.exec())
    {
        while (query.next())
        {
            asignaturas.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toInt()
            });
        }
    }
    else
    {
        Logger::log(Logger::ERROR_LEVEL,
                    QString("Error obteniendo asignaturas del curso %1").arg(academicYearId));
    }

    return asignaturas;
}

bool DataBaseManager::getUserByEmail(const QString& email, UserInfo& usuario, QString* errorMsg)
{
    QSqlQuery query(mDataBase);
    query.prepare("SELECT ID_Usuario, Nombre, Apellido, Email, Contraseña "
                  "FROM Usuario WHERE Email = :email");
    query.bindValue(":email", email);

    if (!executeQuery(query, errorMsg ? *errorMsg : "Error al obtener usuario"))
    {
        if (errorMsg) *errorMsg = "No se pudo buscar el usuario";
        return false;
    }

    if (query.next())
    {
        usuario.id         = query.value(0).toInt();
        usuario.nombre     = query.value(1).toString();
        usuario.apellido   = query.value(2).toString();
        usuario.email      = query.value(3).toString();
        usuario.contrasena = query.value(4).toString();
        return true;
    }

    if (errorMsg) *errorMsg = "Usuario no encontrado";
    return false;
}

bool DataBaseManager::insertUser(const QString& nombre, const QString& apellido,
                                 const QString& email, const QString& contrasena,
                                 const QString& tipoRol, QString* errorMsg)
{
    if (!validateUserFields(nombre, apellido, email, contrasena, tipoRol, errorMsg))
        return false;

    QString hashedPassword = PasswordManager::hashPassword(contrasena);

    QSqlQuery query(mDataBase);
    query.prepare("INSERT INTO Usuario (Nombre, Apellido, Email, Contraseña, Tipo_rol) "
                  "VALUES (:nombre, :apellido, :email, :contrasena, :tipo_rol)");
    query.bindValue(":nombre",    nombre);
    query.bindValue(":apellido",  apellido);
    query.bindValue(":email",     email);
    query.bindValue(":contrasena", hashedPassword);
    query.bindValue(":tipo_rol",  tipoRol);

    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        Logger::log(Logger::ERROR_LEVEL, QString("Error insertando usuario %1").arg(email));
        return false;
    }

    Logger::log(Logger::INFO, QString("Usuario creado: %1").arg(email));
    return true;
}

bool DataBaseManager::insertSubject(const QString& nombre, const QString& descripcion,
                                    int academicYearId, QString* errorMsg)
{
    if (nombre.trimmed().isEmpty())
    {
        if (errorMsg) *errorMsg = "El nombre de la asignatura es requerido";
        return false;
    }

    QSqlQuery query(mDataBase);
    query.prepare("INSERT INTO Asignatura (Nombre, Descripción, ID_CursoAcademico) "
                  "VALUES (:nombre, :descripcion, :id_curso_academico)");
    query.bindValue(":nombre",             nombre);
    query.bindValue(":descripcion",        descripcion);
    query.bindValue(":id_curso_academico", academicYearId);

    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        Logger::log(Logger::ERROR_LEVEL, QString("Error insertando asignatura %1").arg(nombre));
        return false;
    }

    Logger::log(Logger::INFO, QString("Asignatura creada: %1").arg(nombre));
    return true;
}

bool DataBaseManager::insertAcademicYear(const QString& year, QString* errorMsg)
{
    QRegularExpression re("^\\d{2}_\\d{2}$");
    if (!re.match(year).hasMatch())
    {
        if (errorMsg) *errorMsg = "Formato inválido. Usa XX_XX (ej: 25_26)";
        return false;
    }

    QSqlQuery query(mDataBase);
    query.prepare("INSERT INTO CursoAcademico (Año) VALUES (:ano)");
    query.bindValue(":ano", year);

    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        Logger::log(Logger::ERROR_LEVEL, QString("Error insertando año académico %1").arg(year));
        return false;
    }

    Logger::log(Logger::INFO, QString("Año académico creado: %1").arg(year));
    return true;
}

bool DataBaseManager::insertTask(const QString& titulo, const QString& descripcion,
                                 int subjectId, int academicYearId,
                                 const QDateTime& fechaLimite, QString* errorMsg)
{
    if (titulo.trimmed().isEmpty())
    {
        if (errorMsg) *errorMsg = "El título de la tarea es requerido";
        return false;
    }

    QSqlQuery query(mDataBase);
    query.prepare("INSERT INTO Tarea (Titulo, Descripcion, ID_Asignatura, ID_CursoAcademico, FechaLimite) "
                  "VALUES (:titulo, :descripcion, :id_asignatura, :id_curso, :fecha_limite)");
    query.bindValue(":titulo",       titulo);
    query.bindValue(":descripcion",  descripcion);
    query.bindValue(":id_asignatura", subjectId);
    query.bindValue(":id_curso",     academicYearId);
    query.bindValue(":fecha_limite", fechaLimite);

    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        Logger::log(Logger::ERROR_LEVEL, QString("Error insertando tarea %1").arg(titulo));
        return false;
    }

    Logger::log(Logger::INFO, QString("Tarea creada: %1").arg(titulo));
    return true;
}

bool DataBaseManager::updateUser(int id, const QString& nombre, const QString& apellido,
                                 const QString& email, const QString& contrasena,
                                 const QString& tipoRol, QString* errorMsg)
{
    if (!validateUserFields(nombre, apellido, email, contrasena, tipoRol, errorMsg))
        return false;

    QString hashedPassword = PasswordManager::hashPassword(contrasena);

    QMap<QString, QVariant> bindings;
    bindings[":nombre"]     = nombre;
    bindings[":apellido"]   = apellido;
    bindings[":email"]      = email;
    bindings[":contrasena"] = hashedPassword;
    bindings[":tipoRol"]    = tipoRol;
    bindings[":id"]         = id;

    bool result = updateRecord("Usuario",
                               "Nombre = :nombre, Apellido = :apellido, "
                               "Email = :email, Contraseña = :contrasena, Tipo_rol = :tipoRol",
                               "ID_Usuario = :id", bindings);

    if (!result && errorMsg) *errorMsg = "Error actualizando usuario";
    return result;
}

bool DataBaseManager::updateUserKeepingPassword(int id, const QString& nombre, const QString& apellido,
                                                const QString& email, const QString& tipoRol,
                                                QString* errorMsg)
{
    // Misma validación que updateUser salvo la contraseña: aquí no se toca,
    // así que no tiene sentido exigirle longitud mínima ni nada por el estilo.
    if (!validateCommonUserFields(nombre, apellido, email, tipoRol, errorMsg))
        return false;

    QMap<QString, QVariant> bindings;
    bindings[":nombre"]   = nombre;
    bindings[":apellido"] = apellido;
    bindings[":email"]    = email;
    bindings[":tipoRol"]  = tipoRol;
    bindings[":id"]       = id;

    // A propósito, el SET no incluye "Contraseña = ...": así el hash ya
    // almacenado se queda tal cual y el usuario conserva su contraseña.
    bool result = updateRecord("Usuario",
                               "Nombre = :nombre, Apellido = :apellido, "
                               "Email = :email, Tipo_rol = :tipoRol",
                               "ID_Usuario = :id", bindings);

    if (!result && errorMsg) *errorMsg = "Error actualizando usuario";
    return result;
}

bool DataBaseManager::updateSubject(int id, const QString& nombre, const QString& descripcion,
                                    QString* errorMsg)
{
    if (nombre.trimmed().isEmpty())
    {
        if (errorMsg) *errorMsg = "El nombre de la asignatura es requerido";
        return false;
    }

    QMap<QString, QVariant> bindings;
    bindings[":nombre"]     = nombre;
    bindings[":descripcion"] = descripcion;
    bindings[":id"]         = id;

    bool result = updateRecord("Asignatura",
                               "Nombre = :nombre, Descripción = :descripcion",
                               "ID_Asignatura = :id", bindings);

    if (!result && errorMsg) *errorMsg = "Error actualizando asignatura";
    return result;
}

bool DataBaseManager::updateAcademicYear(int id, const QString& year, QString* errorMsg)
{
    QRegularExpression re("^\\d{2}_\\d{2}$");
    if (!re.match(year).hasMatch())
    {
        if (errorMsg) *errorMsg = "Formato inválido. Usa XX_XX (ej: 25_26)";
        return false;
    }

    QMap<QString, QVariant> bindings;
    bindings[":ano"] = year;
    bindings[":id"]  = id;

    bool result = updateRecord("CursoAcademico", "Año = :ano",
                               "ID_CursoAcademico = :id", bindings);

    if (!result && errorMsg) *errorMsg = "Error actualizando año académico";
    return result;
}

bool DataBaseManager::updateTask(int id, const QString& titulo, const QString& descripcion,
                                 const QDateTime& fechaLimite, QString* errorMsg)
{
    if (titulo.trimmed().isEmpty())
    {
        if (errorMsg) *errorMsg = "El título de la tarea es requerido";
        return false;
    }

    QMap<QString, QVariant> bindings;
    bindings[":titulo"]      = titulo;
    bindings[":descripcion"] = descripcion;
    bindings[":fecha_limite"] = fechaLimite;
    bindings[":id"]          = id;

    bool result = updateRecord("Tarea",
                               "Titulo = :titulo, Descripcion = :descripcion, "
                               "FechaLimite = :fecha_limite",
                               "ID_Tarea = :id", bindings);

    if (!result && errorMsg) *errorMsg = "Error actualizando tarea";
    return result;
}

bool DataBaseManager::deleteUser(int id, QString* errorMsg)
{
    bool result = deleteRecord("Usuario", id);
    if (!result && errorMsg) *errorMsg = "Error eliminando usuario";
    return result;
}

bool DataBaseManager::deleteSubject(int id, QString* errorMsg)
{
    bool result = deleteRecord("Asignatura", id);
    if (!result && errorMsg) *errorMsg = "Error eliminando asignatura";
    return result;
}

bool DataBaseManager::deleteAcademicYear(int id, QString* errorMsg)
{
    bool result = deleteRecord("CursoAcademico", id);
    if (!result && errorMsg) *errorMsg = "Error eliminando año académico";
    return result;
}

bool DataBaseManager::deleteTask(int id, QString* errorMsg)
{
    bool result = deleteRecord("Tarea", id);
    if (!result && errorMsg) *errorMsg = "Error eliminando tarea";
    return result;
}

bool DataBaseManager::assignTeacherToSubject(int userId, int subjectId, int academicYearId, QString* errorMsg)
{
    QSqlQuery query(mDataBase);
    
    query.prepare("SELECT Tipo_rol FROM Usuario WHERE ID_Usuario = :uid");
    query.bindValue(":uid", userId);
    if (!query.exec() || !query.next())
    {
        if (errorMsg) *errorMsg = "Usuario no encontrado";
        return false;
    }
    if (query.value(0).toString() != "Profesor")
    {
        if (errorMsg) *errorMsg = "Solo se pueden asignar profesores a asignaturas";
        return false;
    }
    
    query.prepare("INSERT INTO Asignacion_Usuario_Asignatura "
                  "(ID_Usuario, ID_Asignatura, ID_CursoAcademico) "
                  "VALUES (:uid, :sid, :yid) "
                  "ON CONFLICT DO NOTHING");
    query.bindValue(":uid", userId);
    query.bindValue(":sid", subjectId);
    query.bindValue(":yid", academicYearId);
    
    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        return false;
    }
    
    Logger::log(Logger::INFO, QString("Profesor %1 asignado a asignatura %2").arg(userId).arg(subjectId));
    return true;
}

bool DataBaseManager::removeTeacherFromSubject(int userId, int subjectId, int academicYearId, QString* errorMsg)
{
    QSqlQuery query(mDataBase);
    query.prepare("DELETE FROM Asignacion_Usuario_Asignatura "
                  "WHERE ID_Usuario = :uid AND ID_Asignatura = :sid AND ID_CursoAcademico = :yid");
    query.bindValue(":uid", userId);
    query.bindValue(":sid", subjectId);
    query.bindValue(":yid", academicYearId);
    
    if (!query.exec())
    {
        if (errorMsg) *errorMsg = query.lastError().text();
        return false;
    }
    
    Logger::log(Logger::INFO, QString("Profesor %1 desasignado de asignatura %2").arg(userId).arg(subjectId));
    return true;
}

std::vector<UserInfo> DataBaseManager::getTeachersBySubject(int subjectId, int academicYearId)
{
    std::vector<UserInfo> profesores;
    QSqlQuery query(mDataBase);
    
    query.prepare("SELECT u.ID_Usuario, u.Nombre, u.Apellido, u.Email, u.Contraseña, u.Tipo_rol "
                  "FROM Usuario u "
                  "INNER JOIN Asignacion_Usuario_Asignatura aua ON u.ID_Usuario = aua.ID_Usuario "
                  "WHERE aua.ID_Asignatura = :sid AND aua.ID_CursoAcademico = :yid "
                  "ORDER BY u.Apellido, u.Nombre");
    query.bindValue(":sid", subjectId);
    query.bindValue(":yid", academicYearId);
    
    if (query.exec())
    {
        while (query.next())
        {
            profesores.push_back({
                query.value(0).toInt(),
                query.value(1).toString(),
                query.value(2).toString(),
                query.value(3).toString(),
                query.value(4).toString(),
                query.value(5).toString()
            });
        }
    }
    
    return profesores;
}
