#ifndef SUBMISSIONMANAGER_HPP
#define SUBMISSIONMANAGER_HPP

#include <QString>
#include <QStringList>
#include <QObject>
#include <QProgressDialog>
#include <QProcess>

class SubmissionManager : public QObject
{
    Q_OBJECT

public:
    explicit SubmissionManager(QObject* parent = nullptr);
    ~SubmissionManager();

    void processSubmissionFile(int taskId, const QString& filePath);

    QString getCurrentSubmissionPath() const { return mCurrentSubmissionPath; }
    QStringList getExtractedCppFiles() const { return mExtractedCppFiles; }

signals:
    void extractionStarted();
    void extractionFinished(bool success, const QString& message);
    void extractionProgress(int value);

private slots:
    // La extracción con unrar corre en un QProcess asíncrono (ver extractRAR):
    // antes se esperaba con waitForFinished(30000) bloqueando el hilo de UI,
    // lo que dejaba la ventana congelada mientras el RAR se extraía.
    void onExtractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onExtractProcessError(QProcess::ProcessError error);

private:
    void extractRAR(const QString& rarPath, const QString& destination);
    void searchCppFiles(const QString& basePath);
    QString findUnrarExecutable() const;
    void cleanupTemporaryFiles();
    void finishWithError(const QString& message);

    QString mCurrentSubmissionPath;
    QStringList mExtractedCppFiles;
    QProgressDialog* mProgressDialog = nullptr;
    QProcess* mExtractProcess = nullptr;
};

#endif
