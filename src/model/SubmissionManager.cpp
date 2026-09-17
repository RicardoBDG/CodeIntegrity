#include "SubmissionManager.hpp"
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QProcess>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTimer>
#include <QPointer>

SubmissionManager::SubmissionManager(QObject* parent)
    : QObject(parent) {}

SubmissionManager::~SubmissionManager()
{
    cleanupTemporaryFiles();
}

void SubmissionManager::processSubmissionFile(int taskId, const QString& filePath)
{
    // Evita pisar mExtractProcess/mProgressDialog si ya hay una extracción en
    // marcha: sin este guard, una segunda llamada mientras la primera sigue en
    // curso dejaría el QProcess y el diálogo anteriores huérfanos (fuga y
    // diálogo de progreso que nunca se cierra).
    if (mExtractProcess)
    {
        emit extractionFinished(false, "Ya hay una extracción en curso");
        return;
    }

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
        mCurrentSubmissionPath.clear();
        return;
    }

    mProgressDialog = new QProgressDialog("Extrayendo archivos...", nullptr, 0, 100);
    mProgressDialog->setWindowModality(Qt::WindowModal);
    mProgressDialog->show();

    emit extractionStarted();

    // extractRAR() solo LANZA el proceso de unrar y vuelve enseguida: el resto
    // del trabajo (buscar los .cpp, cerrar el diálogo, emitir extractionFinished)
    // ocurre en onExtractProcessFinished/onExtractProcessError cuando el proceso
    // termine, sin bloquear la ventana mientras tanto.
    extractRAR(filePath, mCurrentSubmissionPath);
}

QString SubmissionManager::findUnrarExecutable() const
{
    // Rutas de instalación habituales de WinRAR en Windows. Se quitó la ruta
    // personal "D:\Programas\Winrar\..." que solo existía en un equipo concreto
    // de desarrollo: en cualquier otro PC (incluido el tribunal) no aporta nada
    // y QStandardPaths::findExecutable("unrar") ya cubre el caso de tenerlo en el PATH.
    QStringList unrarPaths = {
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
        finishWithError("WinRAR no encontrado en las rutas estándar");
        return;
    }

    QDir dir(destination);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    mExtractProcess = new QProcess(this);
    connect(mExtractProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SubmissionManager::onExtractProcessFinished);
    connect(mExtractProcess, &QProcess::errorOccurred,
            this, &SubmissionManager::onExtractProcessError);

    // Mismos 30s que antes tenía waitForFinished(30000), pero sin bloquear el
    // hilo de UI: se captura ESTE QProcess concreto (QPointer, no el miembro)
    // para no arriesgarse a matar una extracción distinta si esta ya terminó y
    // mExtractProcess se reutilizó para otra (mismo patrón que en
    // AnalysisCoordinator::executeJPlag/executeMOSS).
    QTimer::singleShot(30000, this, [proceso = QPointer<QProcess>(mExtractProcess)]()
    {
        if (proceso && proceso->state() == QProcess::Running) proceso->kill();
    });

    mExtractProcess->start(unrarPath, QStringList() << "x" << "-y" << rarPath << destination);
}

void SubmissionManager::onExtractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Si onExtractProcessError ya gestionó este mismo fallo (Qt emite
    // errorOccurred() y luego finished() cuando un proceso crashea), no volver
    // a procesar ni a emitir señales por segunda vez.
    if (!mExtractProcess) return;

    if (exitStatus == QProcess::CrashExit)
    {
        finishWithError("El proceso de extracción se cerró inesperadamente");
        return;
    }

    if (exitCode != 0)
    {
        QString errorMsg = QString::fromLocal8Bit(mExtractProcess->readAllStandardError());
        finishWithError("Error en extracción: " + errorMsg);
        return;
    }

    mExtractProcess->deleteLater();
    mExtractProcess = nullptr;

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
    }
    else
    {
        emit extractionFinished(true, QString("Se encontraron %1 archivos C++")
                                    .arg(mExtractedCppFiles.size()));
    }
}

void SubmissionManager::onExtractProcessError(QProcess::ProcessError error)
{
    if (!mExtractProcess) return; // ver comentario equivalente en onExtractProcessFinished

    QString errorMsg;
    switch (error)
    {
        case QProcess::FailedToStart:
            errorMsg = "No se pudo iniciar unrar";
            break;
        case QProcess::Crashed:
            errorMsg = "El proceso de extracción se cerró inesperadamente";
            break;
        case QProcess::Timedout:
            errorMsg = "La extracción tardó demasiado tiempo";
            break;
        default:
            errorMsg = "Error desconocido al extraer el archivo";
    }

    finishWithError(errorMsg);
}

void SubmissionManager::finishWithError(const QString& message)
{
    if (mProgressDialog)
    {
        mProgressDialog->close();
        delete mProgressDialog;
        mProgressDialog = nullptr;
    }

    if (mExtractProcess)
    {
        mExtractProcess->deleteLater();
        mExtractProcess = nullptr;
    }

    cleanupTemporaryFiles();
    emit extractionFinished(false, message);
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
