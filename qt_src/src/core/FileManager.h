#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief 文件管理器
 *
 * 负责文件搜索、验证和组织
 */
class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);

    // 文件搜索
    QStringList searchVideoFiles(const QString &directory, const QString &pattern = "");
    QStringList searchAudioFiles(const QString &directory, const QString &pattern = "");
    QStringList searchDanmakuFiles(const QString &directory, const QString &pattern = "");

    // 文件验证
    bool isValidVideoFile(const QString &filePath) const;
    bool isValidAudioFile(const QString &filePath) const;
    bool isValidDanmakuFile(const QString &filePath) const;

signals:
    void fileFound(const QString &filePath);
    void searchCompleted(const QStringList &files);

private:
    QStringList getSupportedVideoExtensions() const;
    QStringList getSupportedAudioExtensions() const;
    QStringList getSupportedDanmakuExtensions() const;
};

#endif // FILEMANAGER_H