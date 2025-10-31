#include "MergeWorker.h"
#include "core/ConfigManager.h"
#include "utils/Logger.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MergeWorker::MergeWorker(QObject *parent)
    : QObject(parent)
    , m_workerThread(new QThread(this))
    , m_progressTimer(new QTimer(this))
    , m_processTimer(new QTimer(this))
    , m_state(Idle)
    , m_progress(0)
    , m_totalFiles(0)
    , m_processedFiles(0)
    , m_failedFiles(0)
    , m_currentMode(BILIBILI_MODE)
{
    // 设置工作线程
    moveToThread(m_workerThread);

    // 连接线程信号
    connect(m_workerThread, &QThread::started, this, [this]() {
        setState(Running);
        emit started();
        Logger::info("MergeWorker started");
    });

    connect(m_workerThread, &QThread::finished, this, &MergeWorker::handleWorkerFinished);

    // 设置进度更新定时器
    m_progressTimer->setInterval(100); // 100ms更新一次
    connect(m_progressTimer, &QTimer::timeout, this, &MergeWorker::onUpdateProgress);

    // 设置文件处理定时器
    m_processTimer->setInterval(50); // 50ms处理一个文件
    connect(m_processTimer, &QTimer::timeout, this, &MergeWorker::onProcessFile);

    Logger::info("MergeWorker initialized");
}

MergeWorker::~MergeWorker()
{
    if (m_workerThread->isRunning()) {
        stop();
        m_workerThread->quit();
        m_workerThread->wait(3000); // 等待3秒
    }
}

void MergeWorker::startMerge(const QString &directory, MergeMode mode)
{
    if (getState() != Idle) {
        emitError("合并操作已在进行中");
        return;
    }

    m_currentDirectory = directory;
    m_currentMode = mode;

    // 重置状态
    m_progress = 0;
    m_processedFiles = 0;
    m_failedFiles = 0;
    m_totalFiles = 0;
    m_currentStatus = "准备中...";

    // 启动工作线程
    m_workerThread->start();
}

void MergeWorker::pause()
{
    if (getState() == Running) {
        setState(Paused);
        m_progressTimer->stop();
        m_processTimer->stop();
        emit statusChanged("已暂停");
        Logger::info("MergeWorker paused");
    }
}

void MergeWorker::resume()
{
    if (getState() == Paused) {
        setState(Running);
        m_progressTimer->start();
        m_processTimer->start();
        emit statusChanged("合并中...");
        Logger::info("MergeWorker resumed");
    }
}

void MergeWorker::stop()
{
    if (getState() == Running || getState() == Paused) {
        setState(Stopping);
        m_progressTimer->stop();
        m_processTimer->stop();

        // 唤醒暂停的线程
        m_pauseCondition.wakeAll();

        emit statusChanged("正在停止...");
        Logger::info("MergeWorker stopping");
    }
}

void MergeWorker::cancel()
{
    stop();
    setState(Stopped);
    emit statusChanged("已取消");
    emit canceled();
    Logger::info("MergeWorker canceled");
}

bool MergeWorker::isRunning() const
{
    return getState() == Running;
}

bool MergeWorker::isPaused() const
{
    return getState() == Paused;
}

bool MergeWorker::isStopped() const
{
    WorkerState state = getState();
    return state == Stopping || state == Stopped;
}

int MergeWorker::getCurrentProgress() const
{
    return m_progress;
}

QString MergeWorker::getCurrentStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentStatus;
}

int MergeWorker::getTotalFiles() const
{
    return m_totalFiles;
}

int MergeWorker::getProcessedFiles() const
{
    return m_processedFiles;
}

int MergeWorker::getFailedFiles() const
{
    return m_failedFiles;
}

void MergeWorker::onUpdateProgress()
{
    if (getState() != Running) {
        return;
    }

    int progress = getCurrentProgress();
    QString status = getCurrentStatus();

    emit progressUpdated(progress, status);
}

void MergeWorker::onProcessFile()
{
    if (getState() != Running) {
        return;
    }

    // 检查是否需要暂停
    if (getState() == Paused) {
        m_mutex.lock();
        m_pauseCondition.wait(&m_mutex);
        m_mutex.unlock();
    }

    // 检查是否需要停止
    if (getState() == Stopping) {
        m_processTimer->stop();
        setState(Finished);
        emit finished();
        emit mergeCompleted(getProcessedFiles(), getProcessedFiles() - getFailedFiles(), getFailedFiles());
        Logger::info("MergeWorker finished");
        return;
    }

    // 这里添加实际的文件处理逻辑
    // 为了演示，我们模拟进度更新
    static int fileCounter = 0;
    fileCounter++;

    int processed = getProcessedFiles() + 1;
    m_processedFiles = processed;

    if (m_totalFiles == 0) {
        m_totalFiles = 10; // 模拟10个文件
    }

    int progress = (processed * 100) / m_totalFiles;
    m_progress = qMin(progress, 100);

    QString fileName = QString("模拟文件_%1.mp4").arg(fileCounter);
    emit fileProcessed(fileName, true);

    m_currentStatus = QString("正在处理: %1 (%2/%3)")
                     .arg(fileName)
                     .arg(processed)
                     .arg(m_totalFiles);

    if (processed >= m_totalFiles) {
        m_processTimer->stop();
        setState(Finished);
        emit finished();
        emit mergeCompleted(processed, processed - getFailedFiles(), getFailedFiles());
        Logger::info("MergeWorker completed all files");
    }
}

void MergeWorker::processBilibiliMode(const QString &directory)
{
    // TODO: 实现B站模式的具体逻辑
    // 这里应该搜索B站缓存文件，解析entry.json等
    Q_UNUSED(directory)
    Logger::info("Processing Bilibili mode (not implemented yet)");
}

void MergeWorker::processUniversalMode(const QString &directory)
{
    // TODO: 实现通用模式的具体逻辑
    // 这里应该搜索通用音视频文件对
    Q_UNUSED(directory)
    Logger::info("Processing Universal mode (not implemented yet)");
}

void MergeWorker::mergeVideoFile(const QString &videoPath, const QString &audioPath, const QString &outputPath)
{
    // TODO: 实现FFmpeg视频合并
    Q_UNUSED(videoPath)
    Q_UNUSED(audioPath)
    Q_UNUSED(outputPath)
}

void MergeWorker::processDanmakuFile(const QString &danmakuPath, const QString &outputPath)
{
    // TODO: 实现弹幕转换
    Q_UNUSED(danmakuPath)
    Q_UNUSED(outputPath)
}

void MergeWorker::downloadSubtitle(const QString &aid, const QString &cid, const QString &outputPath)
{
    // TODO: 实现字幕下载
    Q_UNUSED(aid)
    Q_UNUSED(cid)
    Q_UNUSED(outputPath)
}

void MergeWorker::downloadCover(const QString &coverUrl, const QString &outputPath)
{
    // TODO: 实现封面下载
    Q_UNUSED(coverUrl)
    Q_UNUSED(outputPath)
}

void MergeWorker::updateProgress(int progress, const QString &message)
{
    m_progress = progress;
    m_currentStatus = message;
    emit progressUpdated(progress, message);
}

void MergeWorker::emitError(const QString &error)
{
    Logger::error(QString("MergeWorker error: %1").arg(error));
    emit errorOccurred(error);
}

void MergeWorker::emitWarning(const QString &warning)
{
    Logger::warning(QString("MergeWorker warning: %1").arg(warning));
    emit warningOccurred(warning);
}

void MergeWorker::setState(WorkerState state)
{
    m_state = state;
}

MergeWorker::WorkerState MergeWorker::getState() const
{
    return static_cast<WorkerState>(m_state.load());
}

void MergeWorker::handleWorkerFinished()
{
    if (getState() == Stopping) {
        setState(Stopped);
        emit stopped();
        Logger::info("MergeWorker stopped");
    }
}

