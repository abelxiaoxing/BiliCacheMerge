#ifndef COVERDOWNLOADER_H
#define COVERDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QByteArray>
#include <QTimer>

class CoverDownloader : public QObject
{
    Q_OBJECT

public:
    explicit CoverDownloader(QObject *parent = nullptr);
    ~CoverDownloader();

    // 下载封面
    bool downloadCover(const QString &coverUrl, const QString &savePath);

signals:
    void downloadProgress(int percent);
    void downloadLog(const QString &message);
    void downloadCompleted(bool success, const QString &filePath);

private slots:
    void onReplyFinished();
    void onReplyReadyRead();
    void onTimeout();

private:
    // 创建输出目录
    bool createOutputDirectory(const QString &filePath);

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_reply;
    QTimer *m_timeoutTimer;
    QFile *m_outputFile;
    int m_totalBytes;
    int m_downloadedBytes;
};

#endif // COVERDOWNLOADER_H
