#include "CoverDownloader.h"
#include <QDebug>
#include <QApplication>
#include <QDataStream>
#include <QFileInfo>

CoverDownloader::CoverDownloader(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_reply(nullptr)
    , m_timeoutTimer(new QTimer(this))
    , m_outputFile(nullptr)
    , m_totalBytes(0)
    , m_downloadedBytes(0)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &CoverDownloader::onTimeout);
}

CoverDownloader::~CoverDownloader()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }

    if (m_outputFile) {
        m_outputFile->close();
        delete m_outputFile;
    }
}

bool CoverDownloader::downloadCover(const QString &coverUrl, const QString &savePath)
{
    if (coverUrl.isEmpty() || savePath.isEmpty()) {
        emit downloadLog("错误：URL或保存路径为空");
        return false;
    }

    emit downloadLog(QString("开始下载封面: %1").arg(coverUrl));

    // 创建输出目录
    if (!createOutputDirectory(savePath)) {
        emit downloadLog("错误：无法创建输出目录");
        return false;
    }

    // 创建输出文件
    m_outputFile = new QFile(savePath);
    if (!m_outputFile->open(QIODevice::WriteOnly)) {
        emit downloadLog(QString("错误：无法创建文件 %1").arg(savePath));
        delete m_outputFile;
        m_outputFile = nullptr;
        return false;
    }

    // 创建网络请求
    QNetworkRequest request;
    request.setUrl(QUrl(coverUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:67.0) Gecko/20100101 Firefox/67.0");
    request.setRawHeader(QByteArray("Accept"), QByteArray("*/*"));
    request.setRawHeader(QByteArray("Referer"), QByteArray("https://www.bilibili.com/"));

    // 发送请求
    m_reply = m_networkManager->get(request);
    if (!m_reply) {
        emit downloadLog("错误：网络请求失败");
        delete m_outputFile;
        m_outputFile = nullptr;
        return false;
    }

    // 连接信号
    connect(m_reply, &QNetworkReply::finished, this, &CoverDownloader::onReplyFinished);
    connect(m_reply, &QIODevice::readyRead, this, &CoverDownloader::onReplyReadyRead);

    // 设置超时（10秒）
    m_timeoutTimer->start(10000);
    m_totalBytes = 0;
    m_downloadedBytes = 0;

    return true;
}

void CoverDownloader::onReplyFinished()
{
    m_timeoutTimer->stop();

    if (m_reply->error() != QNetworkReply::NoError) {
        emit downloadLog(QString("网络错误: %1").arg(m_reply->errorString()));

        if (m_outputFile) {
            m_outputFile->close();
            delete m_outputFile;
            m_outputFile = nullptr;
        }

        m_reply->deleteLater();
        m_reply = nullptr;

        emit downloadCompleted(false, QString());
        return;
    }

    // 检查HTTP状态码
    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 200) {
        emit downloadLog(QString("HTTP错误: %1").arg(statusCode));

        if (m_outputFile) {
            m_outputFile->close();
            delete m_outputFile;
            m_outputFile = nullptr;
        }

        m_reply->deleteLater();
        m_reply = nullptr;

        emit downloadCompleted(false, QString());
        return;
    }

    // 写入剩余数据
    QByteArray data = m_reply->readAll();
    if (!data.isEmpty() && m_outputFile) {
        m_outputFile->write(data);
    }

    // 关闭文件
    if (m_outputFile) {
        m_outputFile->close();
        QString filePath = m_outputFile->fileName();
        delete m_outputFile;
        m_outputFile = nullptr;

        emit downloadLog(QString("封面下载完成: %1").arg(filePath));
        emit downloadCompleted(true, filePath);
    }

    m_reply->deleteLater();
    m_reply = nullptr;
}

void CoverDownloader::onReplyReadyRead()
{
    if (!m_reply || !m_outputFile) {
        return;
    }

    // 读取数据并写入文件
    QByteArray data = m_reply->readAll();
    if (!data.isEmpty()) {
        m_outputFile->write(data);
        m_downloadedBytes += data.size();

        // 更新总字节数（如果已知）
        if (m_totalBytes > 0) {
            int percent = static_cast<int>((m_downloadedBytes * 100.0) / m_totalBytes);
            emit downloadProgress(percent);
        }
    }

    // 检查内容长度
    if (m_totalBytes == 0) {
        QVariant contentLength = m_reply->header(QNetworkRequest::ContentLengthHeader);
        if (contentLength.isValid()) {
            m_totalBytes = contentLength.toInt();
        }
    }
}

void CoverDownloader::onTimeout()
{
    emit downloadLog("下载超时");

    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (m_outputFile) {
        m_outputFile->close();
        delete m_outputFile;
        m_outputFile = nullptr;
    }

    emit downloadCompleted(false, QString());
}

bool CoverDownloader::createOutputDirectory(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        return dir.mkpath(".");
    }

    return true;
}
