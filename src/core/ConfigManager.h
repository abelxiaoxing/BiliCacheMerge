#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>

/**
 * @brief 配置管理器类
 * 负责管理应用程序的所有配置项
 * 对应Python版本的configparser.ConfigParser
 *
 * 配置文件结构:
 * - config.ini (GB18030编码)
 * - 支持自定义路径配置
 * - 用户记录和统计信息
 */
class ConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();

    // 配置加载和保存
    bool loadConfig();
    bool saveConfig();
    void resetToDefaults();

    // 配置项访问方法 - config section
    bool danmu2ass() const;
    void setDanmu2ass(bool enabled);
    int videoNumber() const;
    void setVideoNumber(int number);
    bool coverSave() const;
    void setCoverSave(bool enabled);
    bool ccDown() const;
    void setCcDown(bool enabled);
    bool oneDir() const;
    void setOneDir(bool enabled);
    bool overwrite() const;
    void setOverwrite(bool enabled);
    QString lastPattern() const;
    void setLastPattern(const QString &pattern);
    int fontSize() const;
    void setFontSize(int size);
    double textOpacity() const;
    void setTextOpacity(double opacity);
    double reverseBlank() const;
    void setReverseBlank(double blank);
    int durationMarquee() const;
    void setDurationMarquee(int duration);
    int durationStill() const;
    void setDurationStill(int duration);
    bool isReduceComments() const;
    void setIsReduceComments(bool reduce);

    // 配置项访问方法 - customPath section
    bool customPermission() const;
    void setCustomPermission(bool permission);
    QString ffmpegPath() const;
    void setFfmpegPath(const QString &path);
    QString patternFilePath() const;
    void setPatternFilePath(const QString &path);

    // 配置项访问方法 - record section
    int userRank() const;
    void setUserRank(int rank);
    int totalVideoNum() const;
    void setTotalVideoNum(int num);
    int totalGroupNum() const;
    void setTotalGroupNum(int num);
    double totalUsingTime() const;
    void setTotalUsingTime(double time);

    // 用户统计和等级计算
    void updateUserStats(int addedVideoNum = 1, int addedGroupNum = 0, double addedTimeMinutes = 0);
    int calculateUserRank() const;
    QString getRankDescription(int rank) const;
    QString getRankTitle(int rank) const;
    int getRankProgress() const;  // 获取下一等级的进度百分比

    // 路径相关
    QString configFilePath() const;
    QString defaultPatternPath() const;
    QString defaultFfmpegPath() const;
    QString defaultFfprobePath() const;

signals:
    void configChanged();

private:
    void initializeDefaultPaths();
    void ensureConfigDirectory();
    bool loadConfigFile();
    bool saveConfigFile();

    // 配置文件路径
    QString m_configFilePath;
    QString m_basePath;
    QString m_patternPath;
    QString m_ffmpegPath;
    QString m_ffprobePath;

    // 配置数据
    QMap<QString, QVariant> m_config;
    QMap<QString, QVariant> m_customPath;
    QMap<QString, QVariant> m_record;
};

#endif // CONFIGMANAGER_H