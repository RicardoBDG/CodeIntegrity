#ifndef SUBMISSIONMANAGER_HPP
#define SUBMISSIONMANAGER_HPP

#include <QString>
#include <QStringList>
#include <QObject>
#include <QProgressDialog>

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

private:
    void extractRAR(const QString& rarPath, const QString& destination);
    void searchCppFiles(const QString& basePath);
    QString findUnrarExecutable() const;
    void cleanupTemporaryFiles();

    QString mCurrentSubmissionPath;
    QStringList mExtractedCppFiles;
    QProgressDialog* mProgressDialog = nullptr;
};

#endif
