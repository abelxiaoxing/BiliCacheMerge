#ifndef MERGETHREAD_H
#define MERGETHREAD_H

#include <QThread>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

class ConfigManager;
class FfmpegManager;
class DanmakuConverter;
class SubtitleDownloader;

/**
 * @brief 合并线程类
 * 负责在后台线程中执行视频合并操作
 * 支持暂停/继续/错误跳过机制
 */
class MergeThread : public QThread
{
    Q_OBJECT

public:
    explicit MergeThread(QObject *parent = nullptr);
    ~MergeThread();

    // 合并配置
    struct MergeConfig {
        QString inputPath;          // 输入路径
        QString outputPath;         // 输出路径
        QString patternName;        // 使用的模式名
        bool danmuEnabled;          // 启用弹幕转ASS
        bool coverEnabled;          // 启用封面保存
        bool subtitleEnabled;       // 启用字幕下载
        bool oneDir;                // 单目录模式
        bool ordered;               // 分P编号
        bool overwrite;             // 覆盖模式
        bool errorSkip;             // 错误跳过
    };

    // 设置配置
    void setConfig(const MergeConfig &config);
    void setConfigManager(ConfigManager *manager);
    void setFfmpegManager(FfmpegManager *manager);
    void setDanmakuConverter(DanmakuConverter *converter);
    void setSubtitleDownloader(SubtitleDownloader *downloader);

    // 线程控制
    void pause();
    void resume();
    void stop();

    // 状态查询
    bool isPaused() const { return m_paused; }
    bool isStopped() const { return m_stopped; }
    int getCurrentIndex() const { return m_currentIndex; }
    int getTotalCount() const { return m_totalCount; }

signals:
    void progressUpdated(int current, int total);
    void logMessage(const QString &message);
    void errorOccurred(const QString &error);
    void mergeCompleted(int successCount, int failedCount);
    void fileMerged(const QString &outputPath);
    void statusChanged(const QString &status);

protected:
    void run() override;

private:
    // 合并单个视频文件
    bool mergeSingleVideo(const FileScanner::VideoFile &videoFile, const QString &outputDir);
    bool mergeBLVFiles(const FileScanner::VideoFile &videoFile, const QString &outputPath);
    bool mergeVideoAudio(const FileScanner::VideoFile &videoFile, const QString &outputPath);

    // 辅助功能
    QString generateOutputPath(const FileScanner::VideoFile &videoFile, const QString &baseDir);
    QString cleanFileName(const QString &fileName);
    bool downloadCover(const QString &coverUrl, const QString &outputPath);
    bool downloadSubtitle(const QString &aid, const QString &cid, const QString &outputDir);
    bool convertDanmaku(const QString &danmuPath, const QString &outputPath);

    // 等待和通知
    void waitIfPaused();

    // 状态变量
    MergeConfig m_config;
    ConfigManager *m_configManager;
    FfmpegManager *m_ffmpegManager;
    DanmakuConverter *m_danmakuConverter;
    SubtitleDownloader *m_subtitleDownloader;

    QList<FileScanner::VideoGroup> m_videoGroups;
    int m_currentIndex;
    int m_totalCount;
    int m_successCount;
    int m_failedCount;

    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;
    bool m_paused;
    bool m_stopped;
};

#endif // MERGETHREAD_H
