#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QList>
#include <QString>
#include <QRegularExpression>

class PatternManager;
class ConfigManager;

/**
 * @brief 文件扫描器类
 * 负责扫描指定目录，识别B站缓存文件并匹配模式
 * 对应Python版本的videoSearch函数和mainfunc类
 *
 * 扫描结果结构:
 * - 非分组模式: 单个视频文件组
 * - 分组模式: 视频组（包含多个分集）
 */
class FileScanner : public QObject
{
    Q_OBJECT

public:
    explicit FileScanner(ConfigManager *configManager, PatternManager *patternManager, QObject *parent = nullptr);
    ~FileScanner();

    // 扫描配置
    struct ScanConfig {
        QString searchPath;
        QString patternName;
        bool oneDir;
        bool overwrite;
        bool danmuEnabled;
        bool coverEnabled;
        bool subtitleEnabled;
        bool ordered;
    };

    // 扫描结果结构
    struct VideoFile {
        QString entryPath;
        QString videoPath;
        QString audioPath;
        QString danmuPath;
        QString coverPath;
        QVariantMap metadata;
    };

    struct VideoGroup {
        QString patternName;
        QString groupEntryPath;
        QString coverPath;
        QList<VideoFile> files;
        QVariantMap groupMetadata;
    };

    // 扫描方法
    bool scan(const ScanConfig &config);
    QList<VideoGroup> videoGroups() const;
    int totalFiles() const;
    int totalGroups() const;

signals:
    void scanProgress(int current, int total);
    void scanCompleted(bool success);
    void scanError(const QString &error);
    void scanLog(const QString &message);

private:
    bool scanDirectory(const QString &path, const QVariantMap &pattern);
    bool isEntryFile(const QString &filePath, const QVariantMap &pattern);
    QString resolvePathTemplate(const QString &templateStr, const QString &entryPath, const QVariantMap &pattern);
    QVariantMap extractMetadata(const QString &entryPath, const QVariantMap &parseRules);
    QVariant getValueFromJsonPath(const QJsonObject &jsonObj, const QString &path) const;
    bool validateFilePath(const QString &filePath) const;
    bool validateMediaFile(const QString &filePath) const;
    bool hasValidMediaPair(const VideoFile &videoFile) const;

    ConfigManager* m_configManager;
    PatternManager* m_patternManager;
    ScanConfig m_config;
    QList<VideoGroup> m_videoGroups;
    int m_totalFiles;
    int m_totalGroups;
    QRegularExpression m_invalidCharsRegex;
};

#endif // FILESCANNER_H