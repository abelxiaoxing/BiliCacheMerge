#include "Utils.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QTimer>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QtConcurrent>
#include <QMetaObject>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils()
{
}

bool Utils::copyFile(const QString &source, const QString &destination, qint64 *bytesCopied)
{
    QFile srcFile(source);
    if (!srcFile.exists()) {
        log(QString("源文件不存在: %1").arg(source));
        return false;
    }

    // 确保目标目录存在
    QDir destDir(QFileInfo(destination).absolutePath());
    if (!destDir.exists()) {
        if (!destDir.mkpath(destDir.absolutePath())) {
            log(QString("无法创建目标目录: %1").arg(destDir.absolutePath()));
            return false;
        }
    }

    // 检查是否有部分文件存在（断点续传）
    QFile destFile(destination);
    qint64 startPos = 0;
    if (destFile.exists() && bytesCopied == nullptr) {
        // 如果目标文件存在且未指定断点位置，则删除重新开始
        destFile.remove();
    } else if (destFile.exists()) {
        startPos = destFile.size();
        if (!destFile.open(QIODevice::ReadWrite)) {
            log(QString("无法打开目标文件进行断点续传: %1").arg(destination));
            return false;
        }
    }

    if (!srcFile.open(QIODevice::ReadOnly)) {
        log(QString("无法打开源文件: %1").arg(source));
        return false;
    }

    if (!destFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
        log(QString("无法创建目标文件: %1").arg(destination));
        srcFile.close();
        return false;
    }

    // 跳转到起始位置
    srcFile.seek(startPos);

    const qint64 BufferSize = 65536; // 64KB buffer
    char *buffer = new char[BufferSize];
    qint64 totalBytes = srcFile.size();
    qint64 copiedBytes = startPos;

    while (!srcFile.atEnd()) {
        qint64 bytesToRead = qMin(BufferSize, totalBytes - copiedBytes);
        qint64 bytesRead = srcFile.read(buffer, bytesToRead);

        if (bytesRead < 0) {
            break;
        }

        qint64 bytesWritten = destFile.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            delete[] buffer;
            srcFile.close();
            destFile.close();
            log(QString("写入文件失败，已写入 %1/%2 字节").arg(copiedBytes).arg(totalBytes));
            return false;
        }

        copiedBytes += bytesWritten;

        if (bytesCopied) {
            *bytesCopied = copiedBytes;
        }

        // 检查是否需要写入磁盘
        if (copiedBytes % (BufferSize * 10) == 0) {
            destFile.flush();
        }
    }

    delete[] buffer;
    srcFile.close();
    destFile.close();

    log(QString("文件复制成功: %1 -> %2 (%3 bytes)")
        .arg(QFileInfo(source).fileName())
        .arg(QFileInfo(destination).fileName())
        .arg(copiedBytes));

    return copiedBytes == totalBytes;
}

bool Utils::copyFileWithProgress(const QString &source, const QString &destination,
                                  std::function<void(qint64, qint64)> progressCallback)
{
    QFile srcFile(source);
    if (!srcFile.exists()) {
        return false;
    }

    if (!srcFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDir destDir(QFileInfo(destination).absolutePath());
    if (!destDir.exists() && !destDir.mkpath(destDir.absolutePath())) {
        srcFile.close();
        return false;
    }

    QFile destFile(destination);
    if (!destFile.open(QIODevice::WriteOnly)) {
        srcFile.close();
        return false;
    }

    const qint64 BufferSize = 65536;
    char *buffer = new char[BufferSize];
    qint64 totalBytes = srcFile.size();
    qint64 copiedBytes = 0;

    while (!srcFile.atEnd()) {
        qint64 bytesRead = srcFile.read(buffer, BufferSize);
        if (bytesRead < 0) {
            break;
        }

        qint64 bytesWritten = destFile.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            delete[] buffer;
            srcFile.close();
            destFile.close();
            return false;
        }

        copiedBytes += bytesWritten;

        if (progressCallback) {
            progressCallback(copiedBytes, totalBytes);
        }
    }

    delete[] buffer;
    srcFile.close();
    destFile.close();

    return copiedBytes == totalBytes;
}

bool Utils::copyFileResumable(const QString &source, const QString &destination)
{
    return copyFile(source, destination);
}

void Utils::copyFileAsync(const QString &source, const QString &destination,
                          std::function<void(bool)> completionCallback)
{
    QtConcurrent::run([source, destination, completionCallback]() {
        bool result = copyFile(source, destination);
        if (completionCallback) {
            QMetaObject::invokeMethod(qApp, [completionCallback, result]() {
                completionCallback(result);
            });
        }
    });
}

bool Utils::download(const QUrl &url, const QString &destination,
                     const QNetworkRequest &request,
                     std::function<void(qint64, qint64)> progressCallback)
{
    QNetworkAccessManager manager;
    QNetworkRequest req = request;
    if (req.url().isEmpty()) {
        req.setUrl(url);
    }
    QNetworkReply *reply = manager.get(req);

    // 设置输出文件
    QDir destDir(QFileInfo(destination).absolutePath());
    if (!destDir.exists() && !destDir.mkpath(destDir.absolutePath())) {
        reply->abort();
        reply->deleteLater();
        return false;
    }

    QFile file(destination);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->abort();
        reply->deleteLater();
        return false;
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::downloadProgress,
                     [progressCallback](qint64 bytesReceived, qint64 bytesTotal) {
                         if (progressCallback) {
                             progressCallback(bytesReceived, bytesTotal);
                         }
                     });

    loop.exec();

    QByteArray data = reply->readAll();
    file.write(data);
    file.close();

    bool success = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();

    return success;
}

void Utils::downloadAsync(const QUrl &url, const QString &destination,
                          std::function<void(bool, const QByteArray&)> completionCallback)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(url));

    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        QByteArray data = reply->readAll();
        bool success = (reply->error() == QNetworkReply::NoError);

        if (success) {
            // 保存文件
            QDir destDir(QFileInfo(destination).absolutePath());
            if (destDir.exists() || destDir.mkpath(destDir.absolutePath())) {
                QFile file(destination);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(data);
                    file.close();
                }
            }
        }

        if (completionCallback) {
            completionCallback(success, data);
        }

        reply->deleteLater();
        manager->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::errorOccurred, [completionCallback](QNetworkReply::NetworkError code) {
        Q_UNUSED(code);
        if (completionCallback) {
            completionCallback(false, QByteArray());
        }
    });
}

bool Utils::ensureDirExists(const QString &dirPath)
{
    QDir dir(dirPath);
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(dir.absolutePath());
}

QString Utils::getSafeFileName(const QString &fileName)
{
    QString safeName = fileName;

    // 移除或替换非法字符
    safeName.remove(QRegularExpression("[\\\\/:*?\"<>|]"));

    // 限制长度
    if (safeName.length() > 200) {
        safeName = safeName.left(200);
    }

    // 移除首尾空格和点
    safeName = safeName.trimmed().remove(QRegularExpression("^[.]+|[.]+$"));

    // 如果文件名为空，使用默认名称
    if (safeName.isEmpty()) {
        safeName = "untitled";
    }

    return safeName;
}

QString Utils::formatFileSize(qint64 bytes)
{
    QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024 && unitIndex < units.size() - 1) {
        size /= 1024;
        unitIndex++;
    }

    return QString("%1 %2").arg(size, 0, 'f', unitIndex == 0 ? 0 : 2).arg(units[unitIndex]);
}

#ifdef Q_OS_WINDOWS
void Utils::hideConsole()
{
    HWND hwnd = GetConsoleWindow();
    if (hwnd != nullptr) {
        ShowWindow(hwnd, SW_HIDE);
    }
}

void Utils::showConsole()
{
    HWND hwnd = GetConsoleWindow();
    if (hwnd != nullptr) {
        ShowWindow(hwnd, SW_SHOW);
    }
}
#endif

QString Utils::calculateHash(const QString &str)
{
    return QString(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
}

bool Utils::isFileReadable(const QString &filePath)
{
    QFile file(filePath);
    return file.exists() && file.open(QIODevice::ReadOnly);
}

bool Utils::isFileWritable(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists()) {
        // 检查目录是否可写
        return isFileWritable(info.absolutePath());
    }
    return info.isWritable();
}

void Utils::log(const QString &message)
{
    qDebug() << "[Utils]" << message;
}
