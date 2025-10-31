#include "HttpClient.h"
#include "qtbilimerge.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_timeout(REQUEST_TIMEOUT)
    , m_maxRetries(3)
    , m_currentRetries(0)
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::handleNetworkReply(QNetworkReply *reply)
{
    Q_UNUSED(reply)
}

void HttpClient::get(const QString &url, int timeout)
{
    get(QUrl(url), timeout);
}

void HttpClient::get(const QUrl &url, int timeout)
{
    Q_UNUSED(url)
    Q_UNUSED(timeout)
}

void HttpClient::post(const QString &url, const QByteArray &data, const QString &contentType)
{
    Q_UNUSED(url)
    Q_UNUSED(data)
    Q_UNUSED(contentType)
}

void HttpClient::post(const QUrl &url, const QByteArray &data, const QString &contentType)
{
    Q_UNUSED(url)
    Q_UNUSED(data)
    Q_UNUSED(contentType)
}

void HttpClient::downloadFile(const QString &url, const QString &savePath)
{
    Q_UNUSED(url)
    Q_UNUSED(savePath)
}

void HttpClient::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
}

void HttpClient::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void HttpClient::setMaxRetries(int retries)
{
    m_maxRetries = retries;
}

QByteArray HttpClient::urlEncode(const QString &str)
{
    return str.toUtf8();
}

QString HttpClient::urlDecode(const QByteArray &data)
{
    return QString::fromUtf8(data);
}

