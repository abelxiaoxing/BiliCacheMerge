#include "ConfigManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QMutexLocker>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_configPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
{
    // 确保配置目录存在
    QDir configDir(m_configPath);
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }

    // 设置配置文件路径
    m_configPath = configDir.absoluteFilePath(CONFIG_FILE_NAME);
    m_applicationDir = QCoreApplication::applicationDirPath();

    // 初始化设置
    initializeSettings();
    loadSettings();

    qDebug() << "ConfigManager initialized with config file:" << m_configPath;
}

ConfigManager& ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

void ConfigManager::initializeSettings()
{
    // 创建QSettings实例
    m_settings = std::make_unique<QSettings>(m_configPath, QSettings::IniFormat);

    // Qt6中setIniCodec已被移除，默认使用UTF-8

    // 如果配置文件不存在，创建默认配置
    if (!QFile::exists(m_configPath)) {
        createDefaultConfig();
    } else {
        // 检查是否需要迁移旧设置
        migrateOldSettings();
    }
}

void ConfigManager::createDefaultConfig()
{
    QMutexLocker locker(&m_mutex);

    // [config] 节
    m_settings->beginGroup("config");
    m_settings->setValue("danmu2ass", true);
    m_settings->setValue("videoNumber", false);
    m_settings->setValue("coverSave", false);
    m_settings->setValue("ccdown", false);
    m_settings->setValue("oneDir", false);
    m_settings->setValue("overWrite", false);

    // 弹幕设置
    m_settings->setValue("fontSize", DEFAULT_FONT_SIZE);
    m_settings->setValue("textOpacity", DEFAULT_TEXT_OPACITY);
    m_settings->setValue("reverseBlank", DEFAULT_REVERSE_BLANK);
    m_settings->setValue("durationMarquee", DEFAULT_DURATION_MARQUEE);
    m_settings->setValue("durationStill", DEFAULT_DURATION_STILL);
    m_settings->setValue("isReduceComments", false);
    m_settings->endGroup();

    // [customPath] 节
    m_settings->beginGroup("customPath");
    m_settings->setValue("customPermission", false);
    m_settings->setValue("ffmpegPath", "");
    m_settings->endGroup();

    // [record] 节
    m_settings->beginGroup("record");
    m_settings->setValue("totalUsingTime", 0.0);
    m_settings->setValue("totalVideoNum", 0);
    m_settings->setValue("totalGroupNum", 0);
    m_settings->setValue("userRank", 1);
    m_settings->endGroup();

    // [lastUsed] 节
    m_settings->beginGroup("lastUsed");
    m_settings->setValue("lastPattern", "");
    m_settings->setValue("lastDirectory", "");
    m_settings->endGroup();

    // 立即保存默认配置
    m_settings->sync();
    qDebug() << "Default configuration created at:" << m_configPath;
}

void ConfigManager::loadSettings()
{
    QMutexLocker locker(&m_mutex);

    if (!m_settings) {
        qWarning() << "Settings not initialized!";
        return;
    }

    // 这里可以添加设置加载后的处理逻辑
    qDebug() << "Settings loaded from:" << m_configPath;
    emit settingsLoaded();
}

void ConfigManager::saveSettings()
{
    QMutexLocker locker(&m_mutex);

    if (!m_settings) {
        qWarning() << "Settings not initialized!";
        return;
    }

    m_settings->sync();
    qDebug() << "Settings saved to:" << m_configPath;
    emit settingsSaved();
}

// 弹幕设置
bool ConfigManager::isDanmuConversionEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("danmu2ass", true).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setDanmuConversionEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("danmu2ass", enabled);
    m_settings->endGroup();
    emit configChanged("danmu2ass", enabled);
}

int ConfigManager::getFontSize() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    int value = m_settings->value("fontSize", DEFAULT_FONT_SIZE).toInt();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setFontSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("fontSize", size);
    m_settings->endGroup();
    emit configChanged("fontSize", size);
}

double ConfigManager::getTextOpacity() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    double value = m_settings->value("textOpacity", DEFAULT_TEXT_OPACITY).toDouble();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setTextOpacity(double opacity)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("textOpacity", opacity);
    m_settings->endGroup();
    emit configChanged("textOpacity", opacity);
}

double ConfigManager::getReverseBlank() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    double value = m_settings->value("reverseBlank", DEFAULT_REVERSE_BLANK).toDouble();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setReverseBlank(double blank)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("reverseBlank", blank);
    m_settings->endGroup();
    emit configChanged("reverseBlank", blank);
}

int ConfigManager::getDurationMarquee() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    int value = m_settings->value("durationMarquee", DEFAULT_DURATION_MARQUEE).toInt();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setDurationMarquee(int duration)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("durationMarquee", duration);
    m_settings->endGroup();
    emit configChanged("durationMarquee", duration);
}

int ConfigManager::getDurationStill() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    int value = m_settings->value("durationStill", DEFAULT_DURATION_STILL).toInt();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setDurationStill(int duration)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("durationStill", duration);
    m_settings->endGroup();
    emit configChanged("durationStill", duration);
}

bool ConfigManager::isReduceComments() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("isReduceComments", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setReduceComments(bool reduce)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("isReduceComments", reduce);
    m_settings->endGroup();
    emit configChanged("isReduceComments", reduce);
}

// 合并设置
bool ConfigManager::isVideoNumberingEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("videoNumber", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setVideoNumberingEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("videoNumber", enabled);
    m_settings->endGroup();
    emit configChanged("videoNumber", enabled);
}

bool ConfigManager::isCoverSaveEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("coverSave", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setCoverSaveEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("coverSave", enabled);
    m_settings->endGroup();
    emit configChanged("coverSave", enabled);
}

bool ConfigManager::isCCSubtitleEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("ccdown", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setCCSubtitleEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("ccdown", enabled);
    m_settings->endGroup();
    emit configChanged("ccdown", enabled);
}

bool ConfigManager::isOneDirOutputEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("oneDir", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setOneDirOutputEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("oneDir", enabled);
    m_settings->endGroup();
    emit configChanged("oneDir", enabled);
}

bool ConfigManager::isOverwriteEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    bool value = m_settings->value("overWrite", false).toBool();
    m_settings->endGroup();
    return value;
}

void ConfigManager::setOverwriteEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("config");
    m_settings->setValue("overWrite", enabled);
    m_settings->endGroup();
    emit configChanged("overWrite", enabled);
}

// 路径设置
QString ConfigManager::getFFmpegPath() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("customPath");
    QString path = m_settings->value("ffmpegPath", "").toString();
    m_settings->endGroup();
    return path;
}

void ConfigManager::setFFmpegPath(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("customPath");
    m_settings->setValue("ffmpegPath", path);
    m_settings->endGroup();
    emit configChanged("ffmpegPath", path);
}

bool ConfigManager::isCustomFFmpegEnabled() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("customPath");
    bool enabled = m_settings->value("customPermission", false).toBool();
    m_settings->endGroup();
    return enabled;
}

void ConfigManager::setCustomFFmpegEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("customPath");
    m_settings->setValue("customPermission", enabled);
    m_settings->endGroup();
    emit configChanged("customPermission", enabled);
}

QString ConfigManager::getPatternsPath() const
{
    // 模式文件通常在应用程序目录的patterns子目录下
    return QDir(m_applicationDir).absoluteFilePath("config/patterns");
}

void ConfigManager::setPatternsPath(const QString &path)
{
    Q_UNUSED(path)
    // 模式路径通常是固定的，不需要保存
}

// 用户记录
double ConfigManager::getTotalUsingTime() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    double time = m_settings->value("totalUsingTime", 0.0).toDouble();
    m_settings->endGroup();
    return time;
}

void ConfigManager::setTotalUsingTime(double time)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    m_settings->setValue("totalUsingTime", time);
    m_settings->endGroup();
    emit configChanged("totalUsingTime", time);
}

int ConfigManager::getTotalVideoNum() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    int num = m_settings->value("totalVideoNum", 0).toInt();
    m_settings->endGroup();
    return num;
}

void ConfigManager::setTotalVideoNum(int num)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    m_settings->setValue("totalVideoNum", num);
    m_settings->endGroup();
    emit configChanged("totalVideoNum", num);
}

int ConfigManager::getTotalGroupNum() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    int num = m_settings->value("totalGroupNum", 0).toInt();
    m_settings->endGroup();
    return num;
}

void ConfigManager::setTotalGroupNum(int num)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    m_settings->setValue("totalGroupNum", num);
    m_settings->endGroup();
    emit configChanged("totalGroupNum", num);
}

int ConfigManager::getUserRank() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    int rank = m_settings->value("userRank", 1).toInt();
    m_settings->endGroup();
    return rank;
}

void ConfigManager::setUserRank(int rank)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("record");
    m_settings->setValue("userRank", rank);
    m_settings->endGroup();
    emit configChanged("userRank", rank);
}

// 最近使用
QString ConfigManager::getLastPattern() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("lastUsed");
    QString pattern = m_settings->value("lastPattern", "").toString();
    m_settings->endGroup();
    return pattern;
}

void ConfigManager::setLastPattern(const QString &pattern)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("lastUsed");
    m_settings->setValue("lastPattern", pattern);
    m_settings->endGroup();
    emit configChanged("lastPattern", pattern);
}

QString ConfigManager::getLastDirectory() const
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("lastUsed");
    QString directory = m_settings->value("lastDirectory", "").toString();
    m_settings->endGroup();
    return directory;
}

void ConfigManager::setLastDirectory(const QString &directory)
{
    QMutexLocker locker(&m_mutex);
    m_settings->beginGroup("lastUsed");
    m_settings->setValue("lastDirectory", directory);
    m_settings->endGroup();
    emit configChanged("lastDirectory", directory);
}

// 文件操作
void ConfigManager::sync()
{
    QMutexLocker locker(&m_mutex);
    if (m_settings) {
        m_settings->sync();
    }
}

void ConfigManager::resetToDefaults()
{
    createDefaultConfig();
    loadSettings();
}

// 工具方法
QString ConfigManager::getConfigFilePath() const
{
    return m_configPath;
}

QString ConfigManager::getLogFilePath() const
{
    QDir logDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    return logDir.absoluteFilePath(LOG_FILE_NAME);
}

QString ConfigManager::getOutputDirectory(const QString &baseDirectory) const
{
    if (isOneDirOutputEnabled()) {
        return QDir(baseDirectory).absoluteFilePath(DEFAULT_OUTPUT_DIR);
    } else {
        return baseDirectory;
    }
}

void ConfigManager::migrateOldSettings()
{
    // 这里可以添加从旧版本配置迁移的逻辑
    // 例如从旧的配置文件位置迁移到新位置
    Q_UNUSED(m_settings)
}

void ConfigManager::handleConfigChange()
{
    // 处理配置变更的内部逻辑
}

