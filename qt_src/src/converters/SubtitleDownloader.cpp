#include "SubtitleDownloader.h"

SubtitleDownloader::SubtitleDownloader(QObject *parent)
    : QObject(parent)
{
}

SubtitleDownloader::DownloadResult SubtitleDownloader::downloadSubtitle(const QString &aid, const QString &cid, const QString &outputPath)
{
    Q_UNUSED(aid)
    Q_UNUSED(cid)
    Q_UNUSED(outputPath)
    return NoSubtitle;
}

QString SubtitleDownloader::buildSubtitleUrl(const QString &aid, const QString &cid) const
{
    Q_UNUSED(aid)
    Q_UNUSED(cid)
    return QString();
}

QString SubtitleDownloader::convertToSrt(const QByteArray &subtitleData) const
{
    Q_UNUSED(subtitleData)
    return QString();
}

bool SubtitleDownloader::saveSrtFile(const QString &filePath, const QString &srtContent) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(srtContent)
    return false;
}

