#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QStringList>
#include <QFileInfo>

/**
 * @brief 文件工具类
 *
 * 提供文件操作的静态方法
 */
class FileUtils
{
public:
    FileUtils() = delete;

    // 文件操作
    static bool copyFile(const QString &source, const QString &destination, int bufferSize = 65536);
    static bool moveFile(const QString &source, const QString &destination);
    static bool deleteFile(const QString &filePath);
    static bool createDirectory(const QString &dirPath);
    static QString getUniqueFileName(const QString &filePath);

    // 文件信息
    static bool fileExists(const QString &filePath);
    static qint64 getFileSize(const QString &filePath);
    static QString getFileExtension(const QString &filePath);
    static QString getFileName(const QString &filePath);
    static QString getBaseName(const QString &filePath);

    // 路径操作
    static QString combinePaths(const QString &path1, const QString &path2);
    static QString getAbsolutePath(const QString &relativePath);
    static QString getParentDirectory(const QString &filePath);
    static bool isAbsolutePath(const QString &path);

    // 文件格式检查
    static bool isVideoFile(const QString &filePath);
    static bool isAudioFile(const QString &filePath);
    static bool isDanmakuFile(const QString &filePath);

    // 编码处理
    static QString detectFileEncoding(const QString &filePath);
    static QByteArray readFileToByteArray(const QString &filePath);
    static bool writeByteArrayToFile(const QString &filePath, const QByteArray &data);
};

#endif // FILEUTILS_H