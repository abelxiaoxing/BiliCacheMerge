#include "SubtitleDownloader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QUrl>
#include <QHttpMultiPart>
#include <QDateTime>

SubtitleDownloader::SubtitleDownloader(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_apiReply(nullptr)
    , m_subtitleReply(nullptr)
    , m_timeoutTimer(new QTimer(this))
    , m_currentStep(0)
    , m_totalSteps(0)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &SubtitleDownloader::onTimeout);
}

SubtitleDownloader::~SubtitleDownloader()
{
    cleanup();
}

bool SubtitleDownloader::downloadSubtitles(const QString &aid, const QString &cid,
                                          const QString &outputDir, const QString &baseName)
{
    emit downloadLog(QString("开始下载字幕: aid=%1, cid=%2").arg(aid).arg(cid));

    // 创建输出目录
    QDir dir;
    if (!dir.exists(outputDir)) {
        if (!dir.mkpath(outputDir)) {
            emit downloadLog("错误：无法创建输出目录");
            return false;
        }
    }

    // 请求API获取字幕列表
    m_apiReply = requestApi(aid, cid);
    if (!m_apiReply) {
        emit downloadLog("错误：API请求失败");
        return false;
    }

    // 设置超时（5秒）
    m_timeoutTimer->start(5000);
    m_currentStep = 0;
    m_totalSteps = 1; // 第一步是获取API

    // 保存参数供后续使用
    m_apiReply->setProperty("aid", aid);
    m_apiReply->setProperty("cid", cid);
    m_apiReply->setProperty("outputDir", outputDir);
    m_apiReply->setProperty("baseName", baseName);

    return true;
}

bool SubtitleDownloader::downloadSubtitle(const QString &url, const QString &filePath)
{
    m_subtitleReply = downloadSubtitleData(url);
    if (!m_subtitleReply) {
        return false;
    }

    m_subtitleReply->setProperty("filePath", filePath);
    m_timeoutTimer->start(5000);

    return true;
}

QNetworkReply* SubtitleDownloader::requestApi(const QString &aid, const QString &cid)
{
    QString url = QString("https://api.bilibili.com/x/web-interface/view?aid=%1&cid=%2")
                  .arg(aid).arg(cid);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:67.0) Gecko/20100101 Firefox/67.0");
    request.setRawHeader(QByteArray("Accept"), QByteArray("*/*"));
    request.setRawHeader(QByteArray("Host"), QByteArray("api.bilibili.com"));

    return m_networkManager->get(request);
}

void SubtitleDownloader::onApiReplyFinished()
{
    m_timeoutTimer->stop();

    if (m_apiReply->error() != QNetworkReply::NoError) {
        emit downloadLog(QString("网络错误: %1").arg(m_apiReply->errorString()));
        cleanup();
        return;
    }

    QByteArray data = m_apiReply->readAll();
    QList<SubtitleItem> subtitles = parseApiResponse(data);

    if (subtitles.isEmpty()) {
        emit downloadLog("未发现字幕数据");
        cleanup();
        return;
    }

    emit downloadLog(QString("发现 %1 个字幕").arg(subtitles.size()));

    // 设置总步骤
    m_totalSteps = subtitles.size();
    m_currentStep = 0;

    // 下载第一个字幕
    if (!subtitles.isEmpty()) {
        SubtitleItem firstSubtitle = subtitles.first();
        QString outputDir = m_apiReply->property("outputDir").toString();
        QString baseName = m_apiReply->property("baseName").toString();

        QString filePath;
        if (subtitles.size() > 1) {
            filePath = QString("%1/%2_%3.srt")
                      .arg(outputDir, baseName, firstSubtitle.language);
        } else {
            filePath = QString("%1/%2.srt").arg(outputDir, baseName);
        }

        firstSubtitle.filePath = filePath;

        // 下载字幕数据
        m_subtitleReply = downloadSubtitleData(firstSubtitle.url);
        if (m_subtitleReply) {
            m_subtitleReply->setProperty("filePath", filePath);
            m_subtitleReply->setProperty("subtitles", QVariant::fromValue(subtitles));
            m_subtitleReply->setProperty("currentIndex", 0);
            m_timeoutTimer->start(5000);
        }
    }

    m_apiReply->deleteLater();
    m_apiReply = nullptr;
}

void SubtitleDownloader::onSubtitleReplyFinished()
{
    m_timeoutTimer->stop();

    if (m_subtitleReply->error() != QNetworkReply::NoError) {
        emit downloadLog(QString("下载错误: %1").arg(m_subtitleReply->errorString()));
        cleanup();
        return;
    }

    QByteArray data = m_subtitleReply->readAll();
    QList<SubtitleEntry> entries = parseSubtitleJson(data);

    QString filePath = m_subtitleReply->property("filePath").toString();

    if (entries.isEmpty()) {
        emit downloadLog("字幕数据为空");
        m_subtitleReply->deleteLater();
        m_subtitleReply = nullptr;
        return;
    }

    emit downloadLog(QString("解析到 %1 条字幕").arg(entries.size()));

    // 转换为SRT
    if (convertToSRT(entries, filePath)) {
        emit downloadLog(QString("字幕已保存: %1").arg(filePath));
        emit downloadCompleted(true, filePath);
    } else {
        emit downloadLog("转换失败");
        emit downloadCompleted(false, filePath);
    }

    // 检查是否还有更多字幕需要下载
    QList<SubtitleItem> subtitles = m_subtitleReply->property("subtitles").value<QList<SubtitleItem>>();
    int currentIndex = m_subtitleReply->property("currentIndex").toInt();

    m_subtitleReply->deleteLater();
    m_subtitleReply = nullptr;

    // 下载下一个字幕
    if (currentIndex + 1 < subtitles.size()) {
        SubtitleItem nextSubtitle = subtitles[currentIndex + 1];
        QString outputDir = m_apiReply ? m_apiReply->property("outputDir").toString()
                                       : QFileInfo(filePath).dir().path();
        QString baseName = m_apiReply ? m_apiReply->property("baseName").toString()
                                      : QFileInfo(filePath).baseName();

        QString nextFilePath = QString("%1/%2_%3.srt")
                              .arg(outputDir, baseName, nextSubtitle.language);

        m_subtitleReply = downloadSubtitleData(nextSubtitle.url);
        if (m_subtitleReply) {
            m_subtitleReply->setProperty("filePath", nextFilePath);
            m_subtitleReply->setProperty("subtitles", QVariant::fromValue(subtitles));
            m_subtitleReply->setProperty("currentIndex", currentIndex + 1);
            m_timeoutTimer->start(5000);
        }
    }
}

void SubtitleDownloader::onTimeout()
{
    emit downloadLog("请求超时");
    cleanup();
}

QList<SubtitleItem> SubtitleDownloader::parseApiResponse(const QByteArray &data)
{
    QList<SubtitleItem> subtitles;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        emit downloadLog("JSON解析错误");
        return subtitles;
    }

    QJsonObject root = doc.object();
    int code = root.value("code").toInt();

    if (code != 0) {
        emit downloadLog(QString("API返回错误: code=%1").arg(code));
        return subtitles;
    }

    QJsonObject dataObj = root.value("data").toObject();
    QJsonObject subtitleObj = dataObj.value("subtitle").toObject();
    QJsonArray list = subtitleObj.value("list").toArray();

    if (list.isEmpty()) {
        emit downloadLog("字幕列表为空");
        return subtitles;
    }

    for (const QJsonValue &value : list) {
        QJsonObject item = value.toObject();
        SubtitleItem subtitle;
        subtitle.language = item.value("lan").toString();
        subtitle.url = item.value("subtitle_url").toString();

        if (!subtitle.url.isEmpty()) {
            subtitles.append(subtitle);
        }
    }

    return subtitles;
}

QNetworkReply* SubtitleDownloader::downloadSubtitleData(const QString &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:67.0) Gecko/20100101 Firefox/67.0");

    return m_networkManager->get(request);
}

QList<SubtitleEntry> SubtitleDownloader::parseSubtitleJson(const QByteArray &data)
{
    QList<SubtitleEntry> entries;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        emit downloadLog("字幕JSON解析错误");
        return entries;
    }

    QJsonObject root = doc.object();
    QJsonArray body = root.value("body").toArray();

    for (const QJsonValue &value : body) {
        QJsonObject item = value.toObject();
        SubtitleEntry entry;
        entry.from = item.value("from").toDouble();
        entry.to = item.value("to").toDouble();
        entry.content = item.value("content").toString();

        entries.append(entry);
    }

    return entries;
}

bool SubtitleDownloader::convertToSRT(const QList<SubtitleEntry> &entries, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit downloadLog(QString("无法创建文件: %1").arg(filePath));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (int i = 0; i < entries.size(); ++i) {
        const SubtitleEntry &entry = entries[i];

        // SRT序号
        stream << (i + 1) << "\n";

        // 时间轴
        stream << convertToSRTTime(entry.from) << " --> "
               << convertToSRTTime(entry.to) << "\n";

        // 字幕内容
        stream << entry.content << "\n\n";
    }

    file.close();
    return true;
}

QString SubtitleDownloader::convertToSRTTime(double seconds)
{
    int hours = static_cast<int>(seconds) / 3600;
    seconds -= hours * 3600;

    int minutes = static_cast<int>(seconds) / 60;
    seconds -= minutes * 60;

    int secs = static_cast<int>(seconds);
    int milliseconds = static_cast<int>((seconds - secs) * 1000);

    return QString("%1:%2:%3,%4")
           .arg(hours, 2, 10, QChar('0'))
           .arg(minutes, 2, 10, QChar('0'))
           .arg(secs, 2, 10, QChar('0'))
           .arg(milliseconds, 3, 10, QChar('0'));
}

void SubtitleDownloader::cleanup()
{
    if (m_apiReply) {
        m_apiReply->abort();
        m_apiReply->deleteLater();
        m_apiReply = nullptr;
    }

    if (m_subtitleReply) {
        m_subtitleReply->abort();
        m_subtitleReply->deleteLater();
        m_subtitleReply = nullptr;
    }

    m_timeoutTimer->stop();
}
