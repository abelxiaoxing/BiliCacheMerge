#include "MergeThread.h"
#include "FileScanner.h"
#include "ConfigManager.h"
#include "FfmpegManager.h"
#include "DanmakuConverter.h"
#include "SubtitleDownloader.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCoreApplication>

MergeThread::MergeThread(QObject *parent)
    : QThread(parent)
    , m_configManager(nullptr)
    , m_ffmpegManager(nullptr)
    , m_danmakuConverter(nullptr)
    , m_subtitleDownloader(nullptr)
    , m_currentIndex(0)
    , m_totalCount(0)
    , m_successCount(0)
    , m_failedCount(0)
    , m_paused(false)
    , m_stopped(false)
{
}

MergeThread::~MergeThread()
{
    stop();
    wait();
}

void MergeThread::setConfig(const MergeConfig &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

void MergeThread::setConfigManager(ConfigManager *manager)
{
    m_configManager = manager;
}

void MergeThread::setFfmpegManager(FfmpegManager *manager)
{
    m_ffmpegManager = manager;
}

void MergeThread::setDanmakuConverter(DanmakuConverter *converter)
{
    m_danmakuConverter = converter;
}

void MergeThread::setSubtitleDownloader(SubtitleDownloader *downloader)
{
    m_subtitleDownloader = downloader;
}

void MergeThread::pause()
{
    QMutexLocker locker(&m_mutex);
    m_paused = true;
    emit statusChanged("已暂停");
}

void MergeThread::resume()
{
    QMutexLocker locker(&m_mutex);
    m_paused = false;
    m_waitCondition.wakeAll();
    emit statusChanged("合并中");
}

void MergeThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopped = true;
    m_waitCondition.wakeAll();
}

void MergeThread::run()
{
    m_currentIndex = 0;
    m_successCount = 0;
    m_failedCount = 0;

    emit statusChanged("初始化...");

    // 扫描视频文件
    FileScanner scanner(m_configManager, nullptr);
    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = m_config.inputPath;
    scanConfig.patternName = m_config.patternName;
    scanConfig.oneDir = m_config.oneDir;
    scanConfig.danmuEnabled = m_config.danmuEnabled;
    scanConfig.coverEnabled = m_config.coverEnabled;
    scanConfig.subtitleEnabled = m_config.subtitleEnabled;

    if (!scanner.scan(scanConfig)) {
        emit errorOccurred("扫描视频文件失败");
        return;
    }

    m_videoGroups = scanner.videoGroups();
    m_totalCount = scanner.totalFiles();

    if (m_totalCount == 0) {
        emit logMessage("未找到可合并的视频文件");
        emit mergeCompleted(0, 0);
        return;
    }

    emit logMessage(QString("找到 %1 组共 %2 个文件").arg(m_videoGroups.size()).arg(m_totalCount));

    // 确保输出目录存在
    QDir outputDir(m_config.outputPath);
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    // 处理每个视频组
    for (const FileScanner::VideoGroup &group : m_videoGroups) {
        if (m_stopped) {
            break;
        }

        // 为每组创建目录（如果不是单目录模式）
        QString groupOutputDir = m_config.outputPath;
        if (!m_config.oneDir) {
            QString groupName = group.groupMetadata.value("title").toString();
            if (!groupName.isEmpty()) {
                groupName = cleanFileName(groupName);
                groupOutputDir = QDir(m_config.outputPath).filePath(groupName);
                QDir(groupOutputDir).mkpath(".");
            }
        }

        // 处理组中的每个视频文件
        for (const FileScanner::VideoFile &videoFile : group.files) {
            if (m_stopped) {
                break;
            }

            waitIfPaused();

            QString outputPath = generateOutputPath(videoFile, groupOutputDir);
            emit statusChanged(QString("合并中: %1").arg(QFileInfo(outputPath).baseName()));

            bool success = mergeSingleVideo(videoFile, outputPath);

            m_currentIndex++;
            emit progressUpdated(m_currentIndex, m_totalCount);

            if (success) {
                m_successCount++;
                emit fileMerged(outputPath);
                emit logMessage(QString("✓ %1").arg(QFileInfo(outputPath).fileName()));
            } else {
                m_failedCount++;
                QString errorMsg = QString("✗ %1").arg(QFileInfo(videoFile.entryPath).fileName());
                emit errorOccurred(errorMsg);
                emit logMessage(errorMsg);

                // 如果不跳过错误，则停止
                if (!m_config.errorSkip) {
                    emit logMessage("合并已停止（错误跳过未启用）");
                    break;
                }
            }
        }
    }

    emit mergeCompleted(m_successCount, m_failedCount);
    emit statusChanged("完成");

    // 调用线程完成钩子
    bool overallSuccess = (m_failedCount == 0);
    if (m_completionHook) {
        m_completionHook(overallSuccess);
    }
}

bool MergeThread::mergeSingleVideo(const FileScanner::VideoFile &videoFile, const QString &outputPath)
{
    // 创建输出目录
    QDir outputDir = QFileInfo(outputPath).dir();
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    // 处理封面
    if (m_config.coverEnabled) {
        QString coverPath = QDir(outputPath).filePath("cover.jpg");
        if (QFile::exists(videoFile.coverPath)) {
            QFile::copy(videoFile.coverPath, coverPath);
        } else {
            // 下载封面
            QString coverUrl = videoFile.metadata.value("cover_url").toString();
            if (!coverUrl.isEmpty()) {
                downloadCover(coverUrl, coverPath);
            }
        }
    }

    // 处理弹幕转ASS
    if (m_config.danmuEnabled) {
        QString danmuPath = QDir(outputPath).filePath("danmu.ass");
        convertDanmaku(videoFile.danmuPath, danmuPath);
    }

    // 处理字幕下载
    if (m_config.subtitleEnabled) {
        QString subtitleDir = QDir(outputPath).filePath("subtitles");
        QString aid = videoFile.metadata.value("aid").toString();
        QString cid = videoFile.metadata.value("cid").toString();
        if (!aid.isEmpty() && !cid.isEmpty()) {
            downloadSubtitle(aid, cid, subtitleDir);
        }
    }

    // 合并视频
    if (videoFile.isBlvFormat) {
        return mergeBLVFiles(videoFile, outputPath);
    } else {
        return mergeVideoAudio(videoFile, outputPath);
    }
}

bool MergeThread::mergeBLVFiles(const FileScanner::VideoFile &videoFile, const QString &outputPath)
{
    if (!m_ffmpegManager) {
        emit errorOccurred("FFmpeg管理器未设置");
        return false;
    }

    // BLV文件处理：单个文件或使用concat
    if (videoFile.blvFiles.size() == 1) {
        // 单个BLV文件直接复制
        QString blvPath = videoFile.blvFiles.first();
        double progress = 0.0;
        return m_ffmpegManager->mergeSingleBLV(blvPath, outputPath, progress);
    } else {
        // 多个BLV文件使用concat
        double progress = 0.0;
        return m_ffmpegManager->mergeBLVFiles(videoFile.blvFiles, outputPath, progress);
    }
}

bool MergeThread::mergeVideoAudio(const FileScanner::VideoFile &videoFile, const QString &outputPath)
{
    if (!m_ffmpegManager) {
        emit errorOccurred("FFmpeg管理器未设置");
        return false;
    }

    if (!QFile::exists(videoFile.videoPath)) {
        emit errorOccurred(QString("视频文件不存在: %1").arg(videoFile.videoPath));
        return false;
    }

    if (!QFile::exists(videoFile.audioPath)) {
        emit errorOccurred(QString("音频文件不存在: %1").arg(videoFile.audioPath));
        return false;
    }

    double progress = 0.0;
    return m_ffmpegManager->mergeVideoAudio(videoFile.videoPath, videoFile.audioPath, outputPath, progress);
}

bool MergeThread::mergeAnyFormat(const QString &videoDir, const QString &outputFile)
{
    if (!m_ffmpegManager) {
        emit errorOccurred("FFmpeg管理器未设置");
        return false;
    }

    QDir dir(videoDir);
    if (!dir.exists()) {
        emit errorOccurred(QString("目录不存在: %1").arg(videoDir));
        return false;
    }

    // 扫描目录下的所有媒体文件
    QStringList videoExts = {"mp4", "m4s", "avi", "flv", "mkv", "rmvb", "3gp", "wmv"};
    QStringList audioExts = {"mp3", "m4a", "aac", "wma", "wav", "ape", "flac", "cda"};

    QFileInfoList allFiles = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList videoFiles;
    QStringList audioFiles;

    for (const QFileInfo &fileInfo : allFiles) {
        QString ext = fileInfo.suffix().toLower();
        if (videoExts.contains(ext)) {
            videoFiles << fileInfo.absoluteFilePath();
        } else if (audioExts.contains(ext)) {
            audioFiles << fileInfo.absoluteFilePath();
        }
    }

    if (videoFiles.isEmpty() || audioFiles.isEmpty()) {
        emit errorOccurred("未找到可合并的视频或音频文件");
        return false;
    }

    // 智能配对视频和音频文件
    QString videoPath;
    QString audioPath;
    bool paired = false;

    // 尝试按文件名配对
    for (const QString &vFile : videoFiles) {
        QString baseName = QFileInfo(vFile).baseName();
        for (const QString &aFile : audioFiles) {
            QString aBaseName = QFileInfo(aFile).baseName();
            if (baseName == aBaseName) {
                videoPath = vFile;
                audioPath = aFile;
                paired = true;
                break;
            }
        }
        if (paired) break;
    }

    // 如果没有配对成功，选择最大的视频和音频文件
    if (!paired) {
        qint64 maxVideoSize = 0;
        qint64 maxAudioSize = 0;

        for (const QString &vFile : videoFiles) {
            qint64 size = QFileInfo(vFile).size();
            if (size > maxVideoSize) {
                maxVideoSize = size;
                videoPath = vFile;
            }
        }

        for (const QString &aFile : audioFiles) {
            qint64 size = QFileInfo(aFile).size();
            if (size > maxAudioSize) {
                maxAudioSize = size;
                audioPath = aFile;
            }
        }

        if (videoPath.isEmpty() || audioPath.isEmpty()) {
            emit errorOccurred("无法匹配视频和音频文件");
            return false;
        }

        emit logMessage("警告：未找到匹配的音视频文件，将使用最大的文件进行合并");
    }

    // 检测音频格式，决定是否需要重编码
    QString audioExt = QFileInfo(audioPath).suffix().toLower();
    QStringList losslessFormats = {"flac", "ape", "wav", "cda"};

    QStringList arguments;
    arguments << "-i" << videoPath << "-i" << audioPath;

    if (losslessFormats.contains(audioExt)) {
        // 高质量音频格式需要转码为AAC
        emit logMessage(QString("检测到高质量音频格式 %1，将转换为AAC格式").arg(audioExt));
        arguments << "-c:v" << "copy" << "-c:a" << "aac" << "-strict" << "experimental";
    } else {
        // 普通格式直接复制
        arguments << "-c" << "copy";
    }

    arguments << "-map" << "0:v:0" << "-map" << "1:a:0" << "-y" << outputFile;

    emit logMessage(QString("开始合并: %1 + %2").arg(QFileInfo(videoPath).fileName())
                                                   .arg(QFileInfo(audioPath).fileName()));

    // 执行FFmpeg命令
    QString output, error;
    bool success = m_ffmpegManager->executeFfmpeg(arguments, output, error);

    if (!success) {
        emit errorOccurred(QString("合并失败: %1").arg(error));
        return false;
    }

    emit logMessage(QString("✓ 合并完成: %1").arg(QFileInfo(outputFile).fileName()));
    return true;
}

QString MergeThread::generateOutputPath(const FileScanner::VideoFile &videoFile, const QString &baseDir)
{
    QString title = videoFile.metadata.value("title").toString();
    QString partTitle = videoFile.metadata.value("part_title").toString();

    if (m_config.ordered && !partTitle.isEmpty()) {
        QString partId = videoFile.metadata.value("part_id").toString();
        if (!partId.isEmpty()) {
            partTitle = QString("%1_%2").arg(partId, partTitle);
        }
    }

    if (partTitle.isEmpty()) {
        partTitle = "video";
    }

    partTitle = cleanFileName(partTitle);

    // 处理文件名冲突
    QString finalName = partTitle;
    if (!m_config.overwrite) {
        int counter = 1;
        QString baseName = finalName;
        while (QFile::exists(QDir(baseDir).filePath(finalName + ".mp4"))) {
            finalName = QString("%1(%2)").arg(baseName).arg(counter++);
        }
    }

    return QDir(baseDir).filePath(finalName + ".mp4");
}

QString MergeThread::cleanFileName(const QString &fileName)
{
    QString cleaned = fileName;
    // 移除非法字符
    cleaned.remove(QRegularExpression("[\\\\/:*?\"<>|]"));
    // 限制长度
    if (cleaned.length() > 200) {
        cleaned = cleaned.left(200);
    }
    return cleaned;
}

bool MergeThread::downloadCover(const QString &coverUrl, const QString &outputPath)
{
    Q_UNUSED(coverUrl);
    Q_UNUSED(outputPath);
    // TODO: 实现封面下载
    return true;
}

bool MergeThread::downloadSubtitle(const QString &aid, const QString &cid, const QString &outputDir)
{
    if (!m_subtitleDownloader) {
        return false;
    }

    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString baseName = "subtitle";
    bool success = m_subtitleDownloader->downloadSubtitles(aid, cid, outputDir, baseName);

    return success;
}

bool MergeThread::convertDanmaku(const QString &danmuPath, const QString &outputPath)
{
    if (!m_danmakuConverter || !QFile::exists(danmuPath)) {
        return false;
    }

    QDir outputDir = QFileInfo(outputPath).dir();
    if (!outputDir.exists()) {
        outputDir.mkpath(".");
    }

    DanmakuConverter::DanmakuConfig config;
    config.fontSize = 25;
    config.textOpacity = 0.6;
    config.durationMarquee = 12.0;
    config.durationStill = 6.0;
    config.reverseBlank = 0.67;
    config.reduceComments = false;
    config.stageWidth = 1080;
    config.stageHeight = 720;
    config.fontFace = "sans-serif";

    return m_danmakuConverter->convertToASS(danmuPath, outputPath, config);
}

void MergeThread::waitIfPaused()
{
    QMutexLocker locker(&m_mutex);
    while (m_paused && !m_stopped) {
        m_waitCondition.wait(&m_mutex);
    }
}

void MergeThread::setCompletionHook(std::function<void(bool)> hook)
{
    QMutexLocker locker(&m_mutex);
    m_completionHook = hook;
}

void MergeThread::registerThreadHook(QThread *thread, std::function<void()> hook)
{
    QObject::connect(thread, &QThread::finished, hook);
}
