#ifndef MERGEWORKER_H
#define MERGEWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <QString>
#include <QTimer>

#include "qtbilimerge.h"

/**
 * @brief 视频合并工作线程
 *
 * 负责在后台执行视频合并操作，避免阻塞主界面
 * 支持暂停、继续和取消操作
 */
class MergeWorker : public QObject
{
    Q_OBJECT

public:
    explicit MergeWorker(QObject *parent = nullptr);
    ~MergeWorker();

    // 操作控制
    void startMerge(const QString &directory, MergeMode mode);
    void pause();
    void resume();
    void stop();
    void cancel();

    // 状态查询
    bool isRunning() const;
    bool isPaused() const;
    bool isStopped() const;

    // 进度信息
    int getCurrentProgress() const;
    QString getCurrentStatus() const;
    int getTotalFiles() const;
    int getProcessedFiles() const;
    int getFailedFiles() const;

signals:
    // 生命周期信号
    void started();
    void finished();
    void stopped();
    void canceled();

    // 进度信号
    void progressUpdated(int progress, const QString &message);
    void fileProcessed(const QString &fileName, bool success);
    void statusChanged(const QString &status);

    // 错误信号
    void errorOccurred(const QString &error);
    void warningOccurred(const QString &warning);

    // 统计信号
    void mergeCompleted(int total, int successful, int failed);

public slots:
    void onUpdateProgress();
    void onProcessFile();

private slots:
    void handleWorkerFinished();

private:
    void processBilibiliMode(const QString &directory);
    void processUniversalMode(const QString &directory);
    void mergeVideoFile(const QString &videoPath, const QString &audioPath, const QString &outputPath);
    void processDanmakuFile(const QString &danmakuPath, const QString &outputPath);
    void downloadSubtitle(const QString &aid, const QString &cid, const QString &outputPath);
    void downloadCover(const QString &coverUrl, const QString &outputPath);

    void updateProgress(int progress, const QString &message);
    void emitError(const QString &error);
    void emitWarning(const QString &warning);

    // 状态管理
    enum WorkerState {
        Idle,
        Running,
        Paused,
        Stopping,
        Stopped,
        Finished
    };

    void setState(WorkerState state);
    WorkerState getState() const;

    // 线程控制
    QThread *m_workerThread;
    QTimer *m_progressTimer;
    QTimer *m_processTimer;

    // 状态变量（原子操作保证线程安全）
    std::atomic<int> m_state;
    std::atomic<int> m_progress;
    std::atomic<int> m_totalFiles;
    std::atomic<int> m_processedFiles;
    std::atomic<int> m_failedFiles;

    // 控制变量
    mutable QMutex m_mutex;
    QWaitCondition m_pauseCondition;

    // 工作参数
    QString m_currentDirectory;
    MergeMode m_currentMode;
    QString m_currentStatus;
    QString m_outputDirectory;
};

#endif // MERGEWORKER_H