#ifndef SUBTITLEDOWNLOADER_H
#define SUBTITLEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QFile>
#include <QTextStream>
#include <QTimer>

struct SubtitleItem {
    QString language;      // 语言标识 (zh, en, ja等)
    QString url;          // 字幕URL
    QString filePath;     // 输出文件路径
};

struct SubtitleEntry {
    double from;          // 开始时间（秒）
    double to;            // 结束时间（秒）
    QString content;      // 字幕内容
};

class SubtitleDownloader : public QObject
{
    Q_OBJECT
public:
    explicit SubtitleDownloader(QObject *parent = nullptr);
    ~SubtitleDownloader();

    // 下载字幕
    bool downloadSubtitles(const QString &aid, const QString &cid,
                          const QString &outputDir, const QString &baseName);

    // 下载单个字幕文件
    bool downloadSubtitle(const QString &url, const QString &filePath);

signals:
    void downloadProgress(int percent);
    void downloadLog(const QString &message);
    void downloadCompleted(bool success, const QString &filePath);

private slots:
    void onApiReplyFinished();
    void onSubtitleReplyFinished();
    void onTimeout();

private:
    // API请求
    QNetworkReply* requestApi(const QString &aid, const QString &cid);

    // 解析API响应
    QList<SubtitleItem> parseApiResponse(const QByteArray &data);

    // 下载字幕数据
    QNetworkReply* downloadSubtitleData(const QString &url);

    // 解析字幕JSON
    QList<SubtitleEntry> parseSubtitleJson(const QByteArray &data);

    // 转换为SRT格式
    bool convertToSRT(const QList<SubtitleEntry> &entries, const QString &filePath);

    // 时间转换
    QString convertToSRTTime(double seconds);

    // 清理资源
    void cleanup();

    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_apiReply;
    QNetworkReply *m_subtitleReply;
    QTimer *m_timeoutTimer;
    int m_currentStep;
    int m_totalSteps;
};

#endif // SUBTITLEDOWNLOADER_H
