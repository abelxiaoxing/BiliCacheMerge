#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QByteArray>
#include <QUrl>

// 前向声明常量（在qtbilimerge.h中定义）
#ifndef REQUEST_TIMEOUT
#define REQUEST_TIMEOUT 30000
#endif

/**
 * @brief HTTP客户端
 *
 * 提供简单的HTTP GET/POST请求功能
 */
class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    // GET请求
    void get(const QString &url, int timeout = REQUEST_TIMEOUT);
    void get(const QUrl &url, int timeout = REQUEST_TIMEOUT);

    // POST请求
    void post(const QString &url, const QByteArray &data, const QString &contentType = "application/json");
    void post(const QUrl &url, const QByteArray &data, const QString &contentType = "application/json");

    // 文件下载
    void downloadFile(const QString &url, const QString &savePath);

    // 设置方法
    void setUserAgent(const QString &userAgent);
    void setTimeout(int timeout);
    void setMaxRetries(int retries);

    // 工具方法
    static QByteArray urlEncode(const QString &str);
    static QString urlDecode(const QByteArray &data);

signals:
    void requestFinished(const QByteArray &data, bool success);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath, bool success);
    void errorOccurred(const QString &error);

private slots:
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QString m_userAgent;
    int m_timeout;
    int m_maxRetries;
    int m_currentRetries;
    QString m_downloadPath;
};

#endif // HTTPCLIENT_H