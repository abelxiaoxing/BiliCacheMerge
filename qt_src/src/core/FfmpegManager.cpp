#include "FfmpegManager.h"
#include "core/ConfigManager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>

FfmpegManager::FfmpegManager(ConfigManager *configManager, QObject *parent)
    : QObject(parent)
    , m_configManager(configManager)
    , m_process(nullptr)
    , m_progressTimer(nullptr)
    , m_isMerging(false)
{
    initializePaths();
}

FfmpegManager::~FfmpegManager()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
    if (m_progressTimer) {
        m_progressTimer->stop();
        m_progressTimer->deleteLater();
    }
}

void FfmpegManager::initializePaths()
{
    if (m_configManager) {
        m_ffmpegPath = m_configManager->defaultFfmpegPath();
        m_ffprobePath = m_configManager->defaultFfprobePath();
    }
}

QString FfmpegManager::ffmpegPath() const
{
    return m_ffmpegPath;
}

QString FfmpegManager::ffprobePath() const
{
    return m_ffprobePath;
}

bool FfmpegManager::isValidFfmpegPath() const
{
    QFileInfo fileInfo(m_ffmpegPath);
    return fileInfo.exists() && fileInfo.isExecutable();
}

QString FfmpegManager::ffmpegVersion() const
{
    if (!isValidFfmpegPath()) {
        return QString();
    }

    QProcess process;
    process.start(m_ffmpegPath, QStringList() << "-version");
    if (!process.waitForFinished(5000)) {
        return QString();
    }

    QByteArray output = process.readAllStandardOutput();
    QString versionString = QString::fromUtf8(output);
    if (!versionString.isEmpty()) {
        QStringList lines = versionString.split('\n');
        if (!lines.isEmpty()) {
            return lines.first();
        }
    }
    return QString();
}

bool FfmpegManager::executeFfmpeg(const QStringList &arguments, QString &output, QString &error)
{
    if (!isValidFfmpegPath()) {
        error = tr("FFmpeg路径无效: %1").arg(m_ffmpegPath);
        return false;
    }

    QProcess process;
    process.start(m_ffmpegPath, arguments);
    if (!process.waitForFinished(30000)) { // 30秒超时
        process.kill();
        error = tr("FFmpeg执行超时");
        return false;
    }

    output = QString::fromUtf8(process.readAllStandardOutput());
    error = QString::fromUtf8(process.readAllStandardError());

    return process.exitCode() == 0;
}

bool FfmpegManager::mergeVideoAudio(const QString &videoPath, const QString &audioPath,
                                   const QString &outputPath, double &progress)
{
    if (!isValidFfmpegPath()) {
        emit ffmpegError(tr("FFmpeg路径无效: %1").arg(m_ffmpegPath));
        return false;
    }

    if (!QFile::exists(videoPath)) {
        emit ffmpegError(tr("视频文件不存在: %1").arg(videoPath));
        return false;
    }

    if (!QFile::exists(audioPath)) {
        emit ffmpegError(tr("音频文件不存在: %1").arg(audioPath));
        return false;
    }

    // 确保输出目录存在
    QFileInfo outputInfo(outputPath);
    if (!outputInfo.dir().exists()) {
        if (!outputInfo.dir().mkpath(".")) {
            emit ffmpegError(tr("无法创建输出目录: %1").arg(outputInfo.dir().path()));
            return false;
        }
    }

    // 构建FFmpeg参数
    QStringList arguments;
    arguments << "-i" << videoPath;
    arguments << "-i" << audioPath;
    arguments << "-c" << "copy";
    arguments << "-y"; // 覆盖输出文件
    arguments << outputPath;

    // 创建进程
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, &QProcess::readyReadStandardOutput, this, &FfmpegManager::onProcessReadyRead);
        connect(m_process, &QProcess::readyReadStandardError, this, &FfmpegManager::onProcessReadyRead);
        connect(m_process, &QProcess::finished, this, &FfmpegManager::onProcessFinished);
    }

    // 创建进度定时器
    if (!m_progressTimer) {
        m_progressTimer = new QTimer(this);
        connect(m_progressTimer, &QTimer::timeout, this, &FfmpegManager::onProgressTimerTimeout);
    }

    m_currentOutput.clear();
    m_currentError.clear();
    m_isMerging = true;
    progress = 0.0;

    // 启动FFmpeg进程
    m_process->start(m_ffmpegPath, arguments);
    if (!m_process->waitForStarted(5000)) {
        emit ffmpegError(tr("无法启动FFmpeg进程"));
        m_isMerging = false;
        return false;
    }

    // 启动进度定时器（每100ms更新一次）
    m_progressTimer->start(100);

    emit ffmpegOutput(tr("开始合并: %1 + %2 -> %3").arg(videoPath, audioPath, outputPath));
    return true;
}

void FfmpegManager::onProcessReadyRead()
{
    if (!m_process) return;

    QByteArray output = m_process->readAllStandardOutput();
    QByteArray error = m_process->readAllStandardError();

    QString outputStr = QString::fromUtf8(output);
    QString errorStr = QString::fromUtf8(error);

    if (!outputStr.isEmpty()) {
        m_currentOutput += outputStr;
        emit ffmpegOutput(outputStr);
    }
    if (!errorStr.isEmpty()) {
        m_currentError += errorStr;
        emit ffmpegError(errorStr);

        // 解析进度信息
        parseFfmpegOutput(errorStr);
    }
}

void FfmpegManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    m_progressTimer->stop();
    m_isMerging = false;

    if (exitCode == 0) {
        emit ffmpegOutput(tr("合并完成"));
        emit ffmpegFinished(true);
    } else {
        QString errorMsg = tr("FFmpeg执行失败，退出码: %1").arg(exitCode);
        if (!m_currentError.isEmpty()) {
            errorMsg += "\n" + m_currentError;
        }
        emit ffmpegError(errorMsg);
        emit ffmpegFinished(false);
    }
}

void FfmpegManager::onProgressTimerTimeout()
{
    if (!m_isMerging) return;

    // 这里可以发送进度更新信号
    // 实际进度计算在parseFfmpegOutput中处理
    // emit progressUpdated(currentProgress);
}

void FfmpegManager::parseFfmpegOutput(const QString &output)
{
    // FFmpeg进度输出示例: "time=00:00:15.32 bitrate=1234.5kbits/s speed=1.23x"
    QRegularExpression timeRegex("time=(\\d+):(\\d+):(\\d+)\\.(\\d+)");
    QRegularExpression durationRegex("Duration: (\\d+):(\\d+):(\\d+)\\.(\\d+)");

    // 查找总时长（通常在开始时输出）
    static double totalDuration = 0.0;
    if (totalDuration == 0.0) {
        QRegularExpressionMatch durationMatch = durationRegex.match(output);
        if (durationMatch.hasMatch()) {
            int hours = durationMatch.captured(1).toInt();
            int minutes = durationMatch.captured(2).toInt();
            int seconds = durationMatch.captured(3).toInt();
            int milliseconds = durationMatch.captured(4).left(2).toInt();
            totalDuration = hours * 3600 + minutes * 60 + seconds + milliseconds * 0.01;
        }
    }

    // 查找当前时间
    QRegularExpressionMatch timeMatch = timeRegex.match(output);
    if (timeMatch.hasMatch() && totalDuration > 0) {
        int hours = timeMatch.captured(1).toInt();
        int minutes = timeMatch.captured(2).toInt();
        int seconds = timeMatch.captured(3).toInt();
        int milliseconds = timeMatch.captured(4).left(2).toInt();
        double currentTime = hours * 3600 + minutes * 60 + seconds + milliseconds * 0.01;
        double progress = (currentTime / totalDuration) * 100.0;
        if (progress >= 0 && progress <= 100) {
            emit progressUpdated(progress);
        }
    }
}

double FfmpegManager::calculateProgressFromOutput(const QString &output)
{
    // 这个方法可以用于更复杂的进度计算
    // 目前在parseFfmpegOutput中直接处理
    Q_UNUSED(output);
    return 0.0;
}