#ifndef ANALYSISCOORDINATOR_HPP
#define ANALYSISCOORDINATOR_HPP

#include <QObject>
#include <QProcess>
#include <QProgressDialog>

enum class AnalysisTool
{
    JPlag,
    MOSS,
    None
};

class AnalysisCoordinator : public QObject
{
    Q_OBJECT

    public:
        explicit AnalysisCoordinator(QObject* parent = nullptr);
        ~AnalysisCoordinator();

        void runAnalysis(AnalysisTool tool, const QString& submissionPath);
        void cancelAnalysis();
        AnalysisTool getSelectedTool() const { return mSelectedTool; }

    signals:
        void analysisStarted(AnalysisTool tool);
        void analysisFinished(bool success, const QString& reportUrl);
        void analysisError(const QString& errorMessage);

    private slots:
        void onAnalysisProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onAnalysisProcessError(QProcess::ProcessError error);
        void onMossOutputAvailable();

    private:
        void executeJPlag(const QString& submissionPath);
        void executeMOSS(const QString& submissionPath);
        QString extractMossReportUrl(const QString& output);
        QString createReportDirectory();

        QProcess* mAnalysisProcess = nullptr;
        QProgressDialog* mProgressDialog = nullptr;
        AnalysisTool mSelectedTool = AnalysisTool::None;
        QString mCurrentReportPath;
        QString mMossReportUrl;
        bool mReportOpened = false;
};

#endif
