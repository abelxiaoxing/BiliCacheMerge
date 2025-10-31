#ifndef SUBTITLEDOWNLOADER_H
#define SUBTITLEDOWNLOADER_H

#include <QObject>
#include <QString>

/**
 * @brief 字幕下载器
 *
 * 下载CC字幕并转换为SRT格式
 */
class SubtitleDownloader : public QObject
{
    Q_OBJECT

public:
    explicit SubtitleDownloader(QObject *parent = nullptr);

    // 下载方法
    enum DownloadResult {
        Success = 0,
        NoSubtitle = 1,
        NetworkError = 2,
        NotFound = 3
    };

    DownloadResult downloadSubtitle(const QString &aid, const QString &cid, const QString &outputPath);

signals:
    void downloadStarted(const QString &aid, const QString &cid);
    void downloadProgress(int progress);
    void downloadFinished(const QString &outputFile, DownloadResult result);
    void downloadError(const QString &error);

private:
    QString buildSubtitleUrl(const QString &aid, const QString &cid) const;
    QString convertToSrt(const QByteArray &subtitleData) const;
    bool saveSrtFile(const QString &filePath, const QString &srtContent) const;
};

#endif // SUBTITLEDOWNLOADER_H