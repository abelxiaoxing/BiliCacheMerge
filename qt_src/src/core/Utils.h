#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QNetworkRequest>
#include <functional>

/**
 * @brief 通用工具类
 * 提供文件复制、网络下载、路径处理等通用工具方法
 * 替代Python版本的utils.py功能
 */
class Utils : public QObject
{
    Q_OBJECT

public:
    explicit Utils(QObject *parent = nullptr);
    ~Utils();

    // ==================== 文件操作工具 ====================

    /**
     * @brief 基础文件复制
     * @param source 源文件路径
     * @param destination 目标文件路径
     * @param bytesCopied 已复制的字节数（用于断点续传）
     * @return 是否成功
     */
    static bool copyFile(const QString &source, const QString &destination, qint64 *bytesCopied = nullptr);

    /**
     * @brief 带进度回调的文件复制
     * @param source 源文件路径
     * @param destination 目标文件路径
     * @param progressCallback 进度回调函数 (已复制, 总字节数)
     * @return 是否成功
     */
    static bool copyFileWithProgress(const QString &source, const QString &destination,
                                     std::function<void(qint64, qint64)> progressCallback);

    /**
     * @brief 断点续传复制
     * @param source 源文件路径
     * @param destination 目标文件路径
     * @return 是否成功
     */
    static bool copyFileResumable(const QString &source, const QString &destination);

    /**
     * @brief 异步文件复制
     * @param source 源文件路径
     * @param destination 目标文件路径
     * @param completionCallback 完成回调函数 (是否成功)
     */
    static void copyFileAsync(const QString &source, const QString &destination,
                              std::function<void(bool)> completionCallback);

    // ==================== 网络下载工具 ====================

    /**
     * @brief 通用网络下载
     * @param url 下载URL
     * @param destination 保存路径
     * @param request 网络请求（可选）
     * @param progressCallback 进度回调 (已下载, 总字节数)
     * @return 是否成功
     */
    static bool download(const QUrl &url, const QString &destination,
                         const QNetworkRequest &request = QNetworkRequest(),
                         std::function<void(qint64, qint64)> progressCallback = nullptr);

    /**
     * @brief 异步网络下载
     * @param url 下载URL
     * @param destination 保存路径
     * @param completionCallback 完成回调 (是否成功, 响应数据)
     */
    static void downloadAsync(const QUrl &url, const QString &destination,
                              std::function<void(bool, const QByteArray&)> completionCallback);

    // ==================== 路径工具 ====================

    /**
     * @brief 确保目录存在
     * @param dirPath 目录路径
     * @return 是否成功或已存在
     */
    static bool ensureDirExists(const QString &dirPath);

    /**
     * @brief 获取安全的文件名
     * @param fileName 原始文件名
     * @return 清理后的文件名
     */
    static QString getSafeFileName(const QString &fileName);

    /**
     * @brief 获取文件大小友好的字符串
     * @param bytes 字节数
     * @return 格式化的大小字符串 (如: 1.5 MB)
     */
    static QString formatFileSize(qint64 bytes);

    // ==================== 控制台工具 (Windows) ====================

#ifdef Q_OS_WINDOWS
    /**
     * @brief 隐藏控制台窗口
     */
    static void hideConsole();

    /**
     * @brief 显示控制台窗口
     */
    static void showConsole();
#endif

    // ==================== 其他工具 ====================

    /**
     * @brief 计算字符串的哈希值
     * @param str 输入字符串
     * @return 哈希值
     */
    static QString calculateHash(const QString &str);

    /**
     * @brief 验证文件是否可读
     * @param filePath 文件路径
     * @return 是否可读
     */
    static bool isFileReadable(const QString &filePath);

    /**
     * @brief 验证文件是否可写
     * @param filePath 文件路径
     * @return 是否可写
     */
    static bool isFileWritable(const QString &filePath);

signals:
    void copyProgress(qint64 bytesCopied, qint64 totalBytes);
    void downloadProgress(qint64 bytesDownloaded, qint64 totalBytes);
    void logMessage(const QString &message);

private:
    static void log(const QString &message);
};

#endif // UTILS_H
