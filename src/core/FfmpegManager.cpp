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

bool FfmpegManager::mergeAnyFormat(const QString &inputPath1, const QString &inputPath2,
                                   const QString &outputPath, double &progress)
{
    Q_UNUSED(progress);
    emit ffmpegOutput(QString("开始通用格式合并: %1 + %2").arg(inputPath1, inputPath2));

    // 检测两个文件的类型
    MediaType type1 = detectMediaType(inputPath1);
    MediaType type2 = detectMediaType(inputPath2);

    // 确定哪个是视频，哪个是音频
    QString videoPath, audioPath;
    if (type1 == VideoType && type2 == AudioType) {
        videoPath = inputPath1;
        audioPath = inputPath2;
    } else if (type1 == AudioType && type2 == VideoType) {
        videoPath = inputPath2;
        audioPath = inputPath1;
    } else {
        emit ffmpegError("错误：无法识别的媒体文件类型");
        return false;
    }

    // 获取格式信息
    QString format1 = getMediaFormat(videoPath);
    QString format2 = getMediaFormat(audioPath);

    // 构建FFmpeg参数
    QStringList args;
    args << "-i" << videoPath << "-i" << audioPath;

    // 如果需要转码（无损格式如FLAC、APE、WAV），则指定编码器
    if (needsTranscoding(format1, format2)) {
        emit ffmpegOutput("检测到无损格式，需要转码");
        // 视频编码
        if (format1 == "flac" || format1 == "ape") {
            args << "-c:v" << "libx264" << "-preset" << "medium";
        }
        // 音频编码
        if (format2 == "flac" || format2 == "ape") {
            args << "-c:a" << "aac" << "-b:a" << "128k";
        } else if (format2 == "wav" || format2 == "pcm") {
            args << "-c:a" << "aac" << "-b:a" << "128k";
        }
    } else {
        args << "-c" << "copy"; // 无损复制
    }

    args << "-y" << outputPath;

    // 执行合并
    m_isMerging = true;
    m_progressTimer->start(100);

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyRead, this, &FfmpegManager::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FfmpegManager::onProcessFinished);

    m_process->start(m_ffmpegPath, args);

    if (!m_process->waitForStarted(3000)) {
        emit ffmpegError("启动FFmpeg失败");
        m_isMerging = false;
        return false;
    }

    return true;
}

bool FfmpegManager::mergeBLVFiles(const QStringList &blvFiles, const QString &outputPath, double &progress)
{
    if (blvFiles.isEmpty()) {
        emit ffmpegError("BLV文件列表为空");
        return false;
    }

    emit ffmpegOutput(QString("检测到 %1 个BLV文件").arg(blvFiles.size()));

    // 单个BLV文件：直接转换容器
    if (blvFiles.size() == 1) {
        return mergeSingleBLV(blvFiles.first(), outputPath, progress);
    }

    // 多个BLV文件：使用concat协议合并
    emit ffmpegOutput("使用concat协议合并多个BLV文件");

    // 创建临时concat文件
    QString tempDir = QDir::tempPath();
    QString concatFilePath = tempDir + "/blv_concat_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".txt";

    QFile concatFile(concatFilePath);
    if (!concatFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit ffmpegError("无法创建临时concat文件");
        return false;
    }

    QTextStream out(&concatFile);
    for (const QString &blvFile : blvFiles) {
        out << "file '" << blvFile << "'\n";
    }
    concatFile.close();

    // 执行concat合并
    QStringList args;
    args << "-f" << "concat" << "-safe" << "0" << "-i" << concatFilePath
         << "-c" << "copy" << "-y" << outputPath;

    m_isMerging = true;
    m_progressTimer->start(100);

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyRead, this, &FfmpegManager::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FfmpegManager::onProcessFinished);

    m_process->start(m_ffmpegPath, args);

    bool started = m_process->waitForStarted(3000);
    concatFile.remove(); // 立即删除临时文件

    if (!started) {
        emit ffmpegError("启动FFmpeg失败");
        m_isMerging = false;
        return false;
    }

    return true;
}

bool FfmpegManager::mergeSingleBLV(const QString &blvPath, const QString &outputPath, double &progress)
{
    emit ffmpegOutput("转换单个BLV文件");
    emit ffmpegOutput(QString("输入: %1").arg(blvPath));
    emit ffmpegOutput(QString("输出: %1").arg(outputPath));

    QStringList args;
    args << "-i" << blvPath << "-c" << "copy" << "-y" << outputPath;

    m_isMerging = true;
    m_progressTimer->start(100);

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyRead, this, &FfmpegManager::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FfmpegManager::onProcessFinished);

    m_process->start(m_ffmpegPath, args);

    if (!m_process->waitForStarted(3000)) {
        emit ffmpegError("启动FFmpeg失败");
        m_isMerging = false;
        return false;
    }

    return true;
}

FfmpegManager::MediaType FfmpegManager::detectMediaType(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();

    // 视频格式
    QStringList videoFormats = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg"};
    // 音频格式
    QStringList audioFormats = {"mp3", "wav", "aac", "flac", "ogg", "m4a", "wma", "ape", "cda"};

    if (videoFormats.contains(ext)) {
        return VideoType;
    } else if (audioFormats.contains(ext)) {
        return AudioType;
    }

    return UnknownType;
}

QString FfmpegManager::getMediaFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}

bool FfmpegManager::needsTranscoding(const QString &format1, const QString &format2)
{
    // 无损格式需要转码
    QStringList losslessFormats = {"flac", "ape", "wav", "pcm"};

    return losslessFormats.contains(format1) || losslessFormats.contains(format2);
}

QStringList FfmpegManager::findMatchingFiles(const QStringList &files, const QString &baseName)
{
    QStringList matches;
    for (const QString &file : files) {
        if (file.contains(baseName)) {
            matches.append(file);
        }
    }
    return matches;
}

QString FfmpegManager::extractBaseName(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.baseName();
}
