#include "AnalysisCoordinator.hpp"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPointer>
#include <qdiriterator.h>

namespace {
    constexpr int kJPlagTimeoutMs = 300000; // 5 minutos
    constexpr int kMossTimeoutMs  = 600000; // 10 minutos
}

AnalysisCoordinator::AnalysisCoordinator(QObject* parent)
    : QObject(parent) {}

AnalysisCoordinator::~AnalysisCoordinator()
{
    cancelAnalysis();
}

QString AnalysisCoordinator::createReportDirectory()
{
    QString reportDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    + "/CodeIntegrity/Reports";
    QDir().mkpath(reportDir);

    mCurrentReportPath = reportDir + "/Reporte_" + QString::number(QDateTime::currentSecsSinceEpoch());
    QDir().mkpath(mCurrentReportPath);
    mReportOpened = false;

    return mCurrentReportPath;
}

void AnalysisCoordinator::runAnalysis(AnalysisTool tool, const QString& submissionPath)
{
    // Sin este guard, lanzar un segundo análisis mientras el primero sigue en
    // marcha pisa mAnalysisProcess con el QProcess nuevo: el proceso anterior
    // queda huérfano (nunca se mata ni se limpia) y, peor, su QTimer de timeout
    // puede acabar matando al proceso NUEVO cuando venza (ver executeJPlag/
    // executeMOSS, donde ahora se captura el QProcess concreto con QPointer
    // precisamente para evitar esa confusión).
    if (mAnalysisProcess && mAnalysisProcess->state() != QProcess::NotRunning)
    {
        emit analysisError("Ya hay un análisis en curso");
        return;
    }

    if (submissionPath.isEmpty())
    {
        emit analysisError("Ruta de entregas vacía");
        return;
    }

    mSelectedTool = tool;
    emit analysisStarted(tool);

    if (tool == AnalysisTool::JPlag)
    {
        executeJPlag(submissionPath);
    }
    else if (tool == AnalysisTool::MOSS)
    {
        executeMOSS(submissionPath);
    }
}

void AnalysisCoordinator::executeJPlag(const QString& submissionPath)
{
    createReportDirectory();

    mAnalysisProcess = new QProcess(this);
    connect(mAnalysisProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AnalysisCoordinator::onAnalysisProcessFinished);
    connect(mAnalysisProcess, &QProcess::errorOccurred,
            this, &AnalysisCoordinator::onAnalysisProcessError);

    QString appDir = QCoreApplication::applicationDirPath();
    QString jplagJar = appDir + "/../tools/jplag.jar";
    
    if (!QFile::exists(jplagJar))
    {
        emit analysisError("JPlag no encontrado en: " + jplagJar);
        return;
    }
    
    QStringList args =
    {
        "-jar", QDir::toNativeSeparators(jplagJar),
        "-l", "cpp",
        "-r", QDir::toNativeSeparators(mCurrentReportPath),
        QDir::toNativeSeparators(submissionPath)
    };

    // Se captura ESTE QProcess concreto (no el miembro mAnalysisProcess) para que,
    // si para cuando venza el timeout ya se ha lanzado otro análisis y
    // mAnalysisProcess apunta a un proceso distinto, el timeout no mate el
    // proceso equivocado. QPointer se pone a nullptr solo si el objeto se ha
    // destruido, así que también es seguro si el proceso ya terminó y fue borrado.
    QTimer::singleShot(kJPlagTimeoutMs, this, [proceso = QPointer<QProcess>(mAnalysisProcess)]()
    {
        if (proceso) proceso->kill();
    });

    mAnalysisProcess->start("java", args);
}

void AnalysisCoordinator::executeMOSS(const QString& submissionPath)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString mossScript = appDir + "/../tools/moss.pl";
    
    QString perlPath = "perl";

    if (!QFile::exists(mossScript))
    {
        emit analysisError("MOSS no encontrado en: " + mossScript);
        return;
    }

    mAnalysisProcess = new QProcess(this);
    connect(mAnalysisProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AnalysisCoordinator::onAnalysisProcessFinished);
    connect(mAnalysisProcess, &QProcess::errorOccurred,
            this, &AnalysisCoordinator::onAnalysisProcessError);
    connect(mAnalysisProcess, &QProcess::readyReadStandardOutput,
            this, &AnalysisCoordinator::onMossOutputAvailable);

    QStringList args = {QDir::toNativeSeparators(mossScript), "-l", "cc"};

    QDirIterator it(submissionPath, {"*.cpp", "*.c", "*.cc", "*.cxx", "*.h", "*.hpp"},
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        args << QDir::toNativeSeparators(it.next());
    }

    // Mismo motivo que en executeJPlag: se captura el QProcess concreto, no el
    // miembro, para no arriesgarse a matar un análisis distinto lanzado después.
    QTimer::singleShot(kMossTimeoutMs, this, [proceso = QPointer<QProcess>(mAnalysisProcess)]()
    {
        if (proceso) proceso->kill();
    });

    mAnalysisProcess->start(perlPath, args);
}

QString AnalysisCoordinator::extractMossReportUrl(const QString& output)
{
    QRegularExpression urlRegex("(https?://[^\\s]+)");
    QRegularExpressionMatch match = urlRegex.match(output);
    return match.hasMatch() ? match.captured(1) : "";
}

void AnalysisCoordinator::onMossOutputAvailable()
{
    QString output = QString(mAnalysisProcess->readAllStandardOutput());
    mMossReportUrl = extractMossReportUrl(output);
}

void AnalysisCoordinator::onAnalysisProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Cuando un QProcess crashea, Qt emite errorOccurred(Crashed) Y DESPUÉS
    // finished(): si onAnalysisProcessError ya limpió mAnalysisProcess (lo puso a
    // nullptr), este segundo aviso no debe volver a emitir señales ni a hacer
    // deleteLater() por segunda vez sobre el mismo puntero.
    if (!mAnalysisProcess) return;

    if (mSelectedTool == AnalysisTool::JPlag)
    {
        if (exitCode == 0 && !mReportOpened)
        {
            mReportOpened = true;
            QDesktopServices::openUrl(QUrl("http://localhost:1996"));
            emit analysisFinished(true, "http://localhost:1996");
        }
        else if (exitCode != 0)
        {
            emit analysisError("JPlag falló con código: " + QString::number(exitCode));
        }
    }
    else if (mSelectedTool == AnalysisTool::MOSS)
    {
        if (!mMossReportUrl.isEmpty())
        {
            QDesktopServices::openUrl(QUrl(mMossReportUrl));
            emit analysisFinished(true, mMossReportUrl);
        }
        else
        {
            emit analysisError("MOSS completó pero no se obtuvo URL del reporte");
        }
    }

    mSelectedTool = AnalysisTool::None;

    // El proceso ya ha terminado (con éxito o no): se programa su borrado y se
    // olvida el puntero para que un siguiente runAnalysis() no lo confunda con
    // el proceso anterior, y para que el guard de arriba corte una segunda
    // notificación duplicada (ver comentario al inicio de esta función).
    mAnalysisProcess->deleteLater();
    mAnalysisProcess = nullptr;
}

void AnalysisCoordinator::onAnalysisProcessError(QProcess::ProcessError error)
{
    // Mismo motivo que en onAnalysisProcessFinished: evita procesar dos veces
    // el mismo fallo si Qt emite errorOccurred() y finished() para el mismo
    // proceso (típico en un crash).
    if (!mAnalysisProcess) return;

    QString errorMsg;
    switch (error)
    {
        case QProcess::FailedToStart:
            errorMsg = "No se pudo iniciar el proceso";
            break;
        case QProcess::Crashed:
            errorMsg = "El proceso se cerró inesperadamente";
            break;
        case QProcess::Timedout:
            errorMsg = "El análisis tardó demasiado tiempo";
            break;
        default:
            errorMsg = "Error desconocido";
    }

    emit analysisError(errorMsg);

    mSelectedTool = AnalysisTool::None;
    mAnalysisProcess->deleteLater();
    mAnalysisProcess = nullptr;
}

void AnalysisCoordinator::cancelAnalysis()
{
    if (mAnalysisProcess && mAnalysisProcess->state() == QProcess::Running) mAnalysisProcess->kill();
}
