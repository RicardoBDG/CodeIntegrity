#include "ui/Login.hpp"
#include "model/DataBaseManager.hpp"
#include "ui/StyleManager.hpp"
#include "model/Logger.hpp"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Logger::log(Logger::INFO, "Aplicación iniciada");

    DataBaseManager* dbManager = DataBaseManager::getInstance();

    if (!dbManager->connectToDatabase())
    {
        Logger::log(Logger::ERROR_LEVEL, "No se pudo conectar a la base de datos");
        QMessageBox::critical(nullptr, "Error de Conexión",
                              "No se pudo conectar a la base de datos.\n\n"
                              "Verifica que:\n"
                              "- PostgreSQL esté ejecutandose en localhost:5432\n"
                              "- Las credenciales sean correctas\n"
                              "- La base de datos 'postgres' exista");
        return -1;
    }

    if (!dbManager->createDataBaseTables())
    {
        Logger::log(Logger::ERROR_LEVEL, "No se pudieron crear las tablas de base de datos");
        QMessageBox::critical(nullptr, "Error de Base de Datos", "No se pudieron crear las tablas de base de datos");
        return -1;
    }

    Login login;
    login.show();

    Logger::log(Logger::INFO, "Ventana de login mostrada");

    return a.exec();
}
