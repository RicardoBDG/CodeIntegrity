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

    QTimer::singleShot(kJPlagTimeoutMs, this, [this]()
    {
        if (mAnalysisProcess) mAnalysisProcess->kill();
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

    QTimer::singleShot(kMossTimeoutMs, this, [this]()
    {
        if (mAnalysisProcess) mAnalysisProcess->kill();
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
}

void AnalysisCoordinator::onAnalysisProcessError(QProcess::ProcessError error)
{
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
}

void AnalysisCoordinator::cancelAnalysis()
{
    if (mAnalysisProcess && mAnalysisProcess->state() == QProcess::Running) mAnalysisProcess->kill();
}
