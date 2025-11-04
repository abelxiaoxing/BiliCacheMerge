#ifndef FFMPEGMANAGER_H
#define FFMPEGMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QMap>
#include <QString>

class ConfigManager;

/**
 * @brief FFmpeg管理器类
 * 负责FFmpeg可执行文件的路径管理、版本检测和执行
 * 对应Python版本的Source.ffmpeg()方法
 */
class FfmpegManager : public QObject
{
    Q_OBJECT

public:
    explicit FfmpegManager(ConfigManager *configManager, QObject *parent = nullptr);
    ~FfmpegManager();

    // FFmpeg路径管理
    QString ffmpegPath() const;
    QString ffprobePath() const;
    bool isValidFfmpegPath() const;
    QString ffmpegVersion() const;

    // FFmpeg执行
    bool executeFfmpeg(const QStringList &arguments, QString &output, QString &error);
    bool mergeVideoAudio(const QString &videoPath, const QString &audioPath,
                        const QString &outputPath, double &progress);

signals:
    void ffmpegOutput(const QString &output);
    void ffmpegError(const QString &error);
    void progressUpdated(double progress);
    void ffmpegFinished(bool success);

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProgressTimerTimeout();

private:
    void initializePaths();
    void parseFfmpegOutput(const QString &output);
    double calculateProgressFromOutput(const QString &output);

    ConfigManager* m_configManager;
    QString m_ffmpegPath;
    QString m_ffprobePath;
    QProcess* m_process;
    QTimer* m_progressTimer;
    QString m_currentOutput;
    QString m_currentError;
    bool m_isMerging;
};

#endif // FFMPEGMANAGER_H