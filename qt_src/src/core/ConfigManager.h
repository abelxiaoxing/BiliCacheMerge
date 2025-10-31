#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QDir>
#include <QMutex>
#include <memory>

#include "qtbilimerge.h"

/**
 * @brief 配置管理器
 *
 * 单例模式，负责管理应用程序的所有配置项
 * 包括用户设置、运行记录、路径配置等
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& getInstance();

    // 配置项访问方法
    // 弹幕设置
    bool isDanmuConversionEnabled() const;
    void setDanmuConversionEnabled(bool enabled);

    int getFontSize() const;
    void setFontSize(int size);

    double getTextOpacity() const;
    void setTextOpacity(double opacity);

    double getReverseBlank() const;
    void setReverseBlank(double blank);

    int getDurationMarquee() const;
    void setDurationMarquee(int duration);

    int getDurationStill() const;
    void setDurationStill(int duration);

    bool isReduceComments() const;
    void setReduceComments(bool reduce);

    // 合并设置
    bool isVideoNumberingEnabled() const;
    void setVideoNumberingEnabled(bool enabled);

    bool isCoverSaveEnabled() const;
    void setCoverSaveEnabled(bool enabled);

    bool isCCSubtitleEnabled() const;
    void setCCSubtitleEnabled(bool enabled);

    bool isOneDirOutputEnabled() const;
    void setOneDirOutputEnabled(bool enabled);

    bool isOverwriteEnabled() const;
    void setOverwriteEnabled(bool enabled);

    // 路径设置
    QString getFFmpegPath() const;
    void setFFmpegPath(const QString &path);

    bool isCustomFFmpegEnabled() const;
    void setCustomFFmpegEnabled(bool enabled);

    QString getPatternsPath() const;
    void setPatternsPath(const QString &path);

    // 用户记录
    double getTotalUsingTime() const;
    void setTotalUsingTime(double time);

    int getTotalVideoNum() const;
    void setTotalVideoNum(int num);

    int getTotalGroupNum() const;
    void setTotalGroupNum(int num);

    int getUserRank() const;
    void setUserRank(int rank);

    // 最近使用
    QString getLastPattern() const;
    void setLastPattern(const QString &pattern);

    QString getLastDirectory() const;
    void setLastDirectory(const QString &directory);

    // 文件操作
    void sync();
    void resetToDefaults();

    // 工具方法
    QString getConfigFilePath() const;
    QString getLogFilePath() const;
    QString getOutputDirectory(const QString &baseDirectory) const;

public slots:
    void loadSettings();
    void saveSettings();

signals:
    void configChanged(const QString &key, const QVariant &value);
    void settingsLoaded();
    void settingsSaved();

private slots:
    void handleConfigChange();

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager() = default;

    // 禁用拷贝构造和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void initializeSettings();
    void migrateOldSettings();
    void createDefaultConfig();

    std::unique_ptr<QSettings> m_settings;
    QString m_configPath;
    QString m_applicationDir;
    mutable QMutex m_mutex;
};

#endif // CONFIGMANAGER_H