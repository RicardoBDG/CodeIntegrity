#include "SubmissionManager.hpp"
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QProcess>
#include <QMessageBox>
#include <QCoreApplication>

SubmissionManager::SubmissionManager(QObject* parent)
    : QObject(parent) {}

SubmissionManager::~SubmissionManager()
{
    cleanupTemporaryFiles();
}

void SubmissionManager::processSubmissionFile(int taskId, const QString& filePath)
{
    if (filePath.isEmpty() || !QFile::exists(filePath))
    {
        emit extractionFinished(false, "Archivo no encontrado");
        return;
    }

    if (QFileInfo(filePath).suffix().toLower() != "rar")
    {
        emit extractionFinished(false, "Solo se aceptan archivos .rar");
        return;
    }

    QString tempBase = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    mCurrentSubmissionPath = tempBase + "/CodeIntegrity_" + QString::number(taskId)
                             + "_" + QString::number(QDateTime::currentSecsSinceEpoch());

    if (!QDir().mkpath(mCurrentSubmissionPath))
    {
        emit extractionFinished(false, "No se pudo crear directorio temporal");
        return;
    }

    mProgressDialog = new QProgressDialog("Extrayendo archivos...", nullptr, 0, 100);
    mProgressDialog->setWindowModality(Qt::WindowModal);
    mProgressDialog->show();
    QCoreApplication::processEvents();

    emit extractionStarted();

    extractRAR(filePath, mCurrentSubmissionPath);

    if (mCurrentSubmissionPath.isEmpty())
    {
        // extractRAR() ya ha limpiado y emitido extractionFinished(false, ...) en caso de fallo
        if (mProgressDialog)
        {
            mProgressDialog->close();
            delete mProgressDialog;
            mProgressDialog = nullptr;
        }
        return;
    }

    searchCppFiles(mCurrentSubmissionPath);

    if (mProgressDialog)
    {
        mProgressDialog->close();
        delete mProgressDialog;
        mProgressDialog = nullptr;
    }

    if (mExtractedCppFiles.isEmpty())
    {
        emit extractionFinished(false, "No se encontraron archivos C++");
        cleanupTemporaryFiles();
    } else {
        emit extractionFinished(true, QString("Se encontraron %1 archivos C++")
                                    .arg(mExtractedCppFiles.size()));
    }
}

QString SubmissionManager::findUnrarExecutable() const
{
    QStringList unrarPaths = {
        "D:\\Programas\\Winrar\\unrar.exe",
        "D:\\Programas\\WinRAR\\unrar.exe",
        "C:\\Program Files\\WinRAR\\unrar.exe",
        "C:\\Program Files (x86)\\WinRAR\\unrar.exe"
    };

    for (const QString& path : unrarPaths)
    {
        if (QFile::exists(path)) {
            return path;
        }
    }

    QString fromPath = QStandardPaths::findExecutable("unrar");
    if (!fromPath.isEmpty())
        return fromPath;

    return "";
}

void SubmissionManager::extractRAR(const QString& rarPath, const QString& destination)
{
    QString unrarPath = findUnrarExecutable();
    if (unrarPath.isEmpty())
    {
        emit extractionFinished(false, "WinRAR no encontrado en las rutas estándar");
        cleanupTemporaryFiles();
        return;
    }

    QDir dir(destination);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    QProcess proceso;
    proceso.start(unrarPath, QStringList() << "x" << "-y" << rarPath << destination);

    if (!proceso.waitForFinished(30000))
    {
        emit extractionFinished(false, "Timeout en la extracción del RAR");
        cleanupTemporaryFiles();
        return;
    }

    if (proceso.exitCode() != 0)
    {
        QString errorMsg = QString::fromLocal8Bit(proceso.readAllStandardError());
        emit extractionFinished(false, "Error en extracción: " + errorMsg);
        cleanupTemporaryFiles();
        return;
    }
}

void SubmissionManager::searchCppFiles(const QString& basePath)
{
    mExtractedCppFiles.clear();

    QDirIterator it(basePath,
                    {"*.cpp", "*.c", "*.cc", "*.cxx", "*.h", "*.hpp"},
                    QDir::Files,
                    QDirIterator::Subdirectories);

    int fileCount = 0;
    while (it.hasNext())
    {
        mExtractedCppFiles << it.next();
        fileCount++;

        if (fileCount % 10 == 0 && mProgressDialog)
        {
            QCoreApplication::processEvents();
        }
    }
}

void SubmissionManager::cleanupTemporaryFiles()
{
    if (!mCurrentSubmissionPath.isEmpty())
    {
        QDir dir(mCurrentSubmissionPath);
        if (dir.exists())
        {
            if (!dir.removeRecursively())
            {
                qWarning() << "No se pudo limpiar:" << mCurrentSubmissionPath;
            }
        }
        mCurrentSubmissionPath.clear();
        mExtractedCppFiles.clear();
    }
}
