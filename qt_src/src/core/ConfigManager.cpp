#include "ConfigManager.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    // 初始化默认路径
    initializeDefaultPaths();

    // 确保配置目录存在
    ensureConfigDirectory();

    // 加载配置文件
    loadConfig();
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::initializeDefaultPaths()
{
    // 获取应用程序基础路径
    m_basePath = QCoreApplication::applicationDirPath();

    // 设置默认路径
    m_patternPath = QDir::cleanPath(m_basePath + "/../../../pattern");
    m_ffmpegPath = QDir::cleanPath(m_basePath + "/../../../python_src/ffmpeg/ffmpeg");
    m_ffprobePath = QDir::cleanPath(m_basePath + "/../../../python_src/ffmpeg/ffprobe");
    m_logPath = QDir::cleanPath(m_basePath + "/../../python_src/data/m4sMerge_bili.log");

    // 配置文件路径 (使用标准配置目录)
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_configFilePath = configDir + "/config.ini";
}

void ConfigManager::ensureConfigDirectory()
{
    QFileInfo fileInfo(m_configFilePath);
    if (!fileInfo.dir().exists()) {
        fileInfo.dir().mkpath(".");
    }
}

bool ConfigManager::loadConfig()
{
    // 如果配置文件不存在，创建默认配置
    if (!QFile::exists(m_configFilePath)) {
        resetToDefaults();
        return saveConfig();
    }

    return loadConfigFile();
}

bool ConfigManager::saveConfig()
{
    return saveConfigFile();
}

void ConfigManager::resetToDefaults()
{
    // config section
    m_config["danmu2ass"] = true;
    m_config["videonumber"] = 0;
    m_config["coversave"] = false;
    m_config["ccdown"] = false;
    m_config["onedir"] = false;
    m_config["overwrite"] = false;
    m_config["lastpattern"] = "";
    m_config["fontsize"] = 23;
    m_config["textopacity"] = 0.6;
    m_config["reverseblank"] = 0.67;
    m_config["durationmarquee"] = 12;
    m_config["durationstill"] = 6;
    m_config["isreducecomments"] = false;

    // customPath section
    m_customPath["custompermission"] = false;
    m_customPath["ffmpegpath"] = "";
    m_customPath["patternfilepath"] = "";

    // record section
    m_record["userrank"] = 1;
    m_record["totalvideonum"] = 0;
    m_record["totalgroupnum"] = 0;
    m_record["totalusingtime"] = 0.0;

    emit configChanged();
}

bool ConfigManager::loadConfigFile()
{
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << m_configFilePath;
        return false;
    }

    // 读取文件内容
    QByteArray fileData = file.readAll();
    file.close();

    // 尝试检测编码并转换为UTF-8
    QString fileContent;
    if (fileData.startsWith("\xFF\xFE")) {
        // UTF-16 LE
        fileContent = QString::fromUtf16(reinterpret_cast<const char16_t*>(fileData.constData() + 2));
    } else if (fileData.startsWith("\xFE\xFF")) {
        // UTF-16 BE
        fileContent = QString::fromUtf16(reinterpret_cast<const char16_t*>(fileData.constData() + 2));
    } else if (fileData.startsWith("\xEF\xBB\xBF")) {
        // UTF-8 with BOM
        fileContent = QString::fromUtf8(fileData.constData() + 3);
    } else {
        // 假设为UTF-8或本地编码
        fileContent = QString::fromLocal8Bit(fileData);
    }

    QString currentSection;
    QStringList lines = fileContent.split('\n');

    for (const QString &lineStr : lines) {
        QString line = lineStr.trimmed();

        // 跳过注释和空行
        if (line.isEmpty() || line.startsWith(';')) {
            continue;
        }

        // 检查是否是新的section
        if (line.startsWith('[') && line.endsWith(']')) {
            currentSection = line.mid(1, line.length() - 2);
            continue;
        }

        // 解析键值对
        int equalsPos = line.indexOf('=');
        if (equalsPos == -1) {
            continue;
        }

        QString key = line.left(equalsPos).trimmed().toLower();
        QString value = line.mid(equalsPos + 1).trimmed();

        // 转换值类型
        QVariant convertedValue;
        if (value == "1" || value == "0") {
            convertedValue = (value == "1");
        } else if (value.contains('.')) {
            convertedValue = value.toDouble();
        } else {
            bool ok;
            int intValue = value.toInt(&ok);
            if (ok) {
                convertedValue = intValue;
            } else {
                convertedValue = value;
            }
        }

        // 存储到对应的section
        if (currentSection == "config") {
            m_config[key] = convertedValue;
        } else if (currentSection == "customPath") {
            m_customPath[key] = convertedValue;
        } else if (currentSection == "record") {
            m_record[key] = convertedValue;
        }
    }

    emit configChanged();
    return true;
}

bool ConfigManager::saveConfigFile()
{
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save config file:" << m_configFilePath;
        return false;
    }

    // 写入文件头
    file.write(";config file of program 'm4sMerge_bili'\n");
    file.write(";version: 1.0\n\n");
    file.write(";DON'T edit unless you kown its effects\n\n");

    // 写入config section
    file.write("[config]\n");
    for (auto it = m_config.begin(); it != m_config.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        // 转换键名为原始格式
        QString originalKey = key;
        if (key == "danmu2ass") originalKey = "danmu2ass";
        else if (key == "videonumber") originalKey = "videonumber";
        else if (key == "coversave") originalKey = "coversave";
        else if (key == "ccdown") originalKey = "ccdown";
        else if (key == "onedir") originalKey = "onedir";
        else if (key == "overwrite") originalKey = "overwrite";
        else if (key == "lastpattern") originalKey = "lastpattern";
        else if (key == "fontsize") originalKey = "fontsize";
        else if (key == "textopacity") originalKey = "textopacity";
        else if (key == "reverseblank") originalKey = "reverseblank";
        else if (key == "durationmarquee") originalKey = "durationmarquee";
        else if (key == "durationstill") originalKey = "durationstill";
        else if (key == "isreducecomments") originalKey = "isreducecomments";

        // 写入值
        if (value.type() == QVariant::Bool) {
            file.write(QString(originalKey + "=" + (value.toBool() ? "1" : "0") + "\n").toLocal8Bit());
        } else if (value.type() == QVariant::Double) {
            file.write(QString(originalKey + "=" + QString::number(value.toDouble(), 'g', 10) + "\n").toLocal8Bit());
        } else {
            file.write(QString(originalKey + "=" + value.toString() + "\n").toLocal8Bit());
        }
    }
    file.write("\n");

    // 写入customPath section
    file.write("[customPath]\n");
    for (auto it = m_customPath.begin(); it != m_customPath.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        QString originalKey = key;
        if (key == "custompermission") originalKey = "custompermission";
        else if (key == "ffmpegpath") originalKey = "ffmpegpath";
        else if (key == "patternfilepath") originalKey = "patternfilepath";

        if (value.type() == QVariant::Bool) {
            file.write(QString(originalKey + "=" + (value.toBool() ? "1" : "0") + "\n").toLocal8Bit());
        } else {
            file.write(QString(originalKey + "=" + value.toString() + "\n").toLocal8Bit());
        }
    }
    file.write("\n");

    // 写入record section
    file.write("[record]\n");
    for (auto it = m_record.begin(); it != m_record.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        QString originalKey = key;
        if (key == "userrank") originalKey = "userrank";
        else if (key == "totalvideonum") originalKey = "totalvideonum";
        else if (key == "totalgroupnum") originalKey = "totalgroupnum";
        else if (key == "totalusingtime") originalKey = "totalusingtime";

        if (value.type() == QVariant::Double) {
            file.write(QString(originalKey + "=" + QString::number(value.toDouble(), 'g', 10) + "\n").toLocal8Bit());
        } else {
            file.write(QString(originalKey + "=" + value.toString() + "\n").toLocal8Bit());
        }
    }
    file.write("\n");

    file.close();
    return true;
}

// config section getters and setters
bool ConfigManager::danmu2ass() const { return m_config.value("danmu2ass", true).toBool(); }
void ConfigManager::setDanmu2ass(bool enabled) { m_config["danmu2ass"] = enabled; emit configChanged(); }

int ConfigManager::videoNumber() const { return m_config.value("videonumber", 0).toInt(); }
void ConfigManager::setVideoNumber(int number) { m_config["videonumber"] = number; emit configChanged(); }

bool ConfigManager::coverSave() const { return m_config.value("coversave", false).toBool(); }
void ConfigManager::setCoverSave(bool enabled) { m_config["coversave"] = enabled; emit configChanged(); }

bool ConfigManager::ccDown() const { return m_config.value("ccdown", false).toBool(); }
void ConfigManager::setCcDown(bool enabled) { m_config["ccdown"] = enabled; emit configChanged(); }

bool ConfigManager::oneDir() const { return m_config.value("onedir", false).toBool(); }
void ConfigManager::setOneDir(bool enabled) { m_config["onedir"] = enabled; emit configChanged(); }

bool ConfigManager::overwrite() const { return m_config.value("overwrite", false).toBool(); }
void ConfigManager::setOverwrite(bool enabled) { m_config["overwrite"] = enabled; emit configChanged(); }

QString ConfigManager::lastPattern() const { return m_config.value("lastpattern", "").toString(); }
void ConfigManager::setLastPattern(const QString &pattern) { m_config["lastpattern"] = pattern; emit configChanged(); }

int ConfigManager::fontSize() const { return m_config.value("fontsize", 23).toInt(); }
void ConfigManager::setFontSize(int size) { m_config["fontsize"] = size; emit configChanged(); }

double ConfigManager::textOpacity() const { return m_config.value("textopacity", 0.6).toDouble(); }
void ConfigManager::setTextOpacity(double opacity) { m_config["textopacity"] = opacity; emit configChanged(); }

double ConfigManager::reverseBlank() const { return m_config.value("reverseblank", 0.67).toDouble(); }
void ConfigManager::setReverseBlank(double blank) { m_config["reverseblank"] = blank; emit configChanged(); }

int ConfigManager::durationMarquee() const { return m_config.value("durationmarquee", 12).toInt(); }
void ConfigManager::setDurationMarquee(int duration) { m_config["durationmarquee"] = duration; emit configChanged(); }

int ConfigManager::durationStill() const { return m_config.value("durationstill", 6).toInt(); }
void ConfigManager::setDurationStill(int duration) { m_config["durationstill"] = duration; emit configChanged(); }

bool ConfigManager::isReduceComments() const { return m_config.value("isreducecomments", false).toBool(); }
void ConfigManager::setIsReduceComments(bool reduce) { m_config["isreducecomments"] = reduce; emit configChanged(); }

// customPath section getters and setters
bool ConfigManager::customPermission() const { return m_customPath.value("custompermission", false).toBool(); }
void ConfigManager::setCustomPermission(bool permission) { m_customPath["custompermission"] = permission; emit configChanged(); }

QString ConfigManager::ffmpegPath() const { return m_customPath.value("ffmpegpath", "").toString(); }
void ConfigManager::setFfmpegPath(const QString &path) { m_customPath["ffmpegpath"] = path; emit configChanged(); }

QString ConfigManager::patternFilePath() const { return m_customPath.value("patternfilepath", "").toString(); }
void ConfigManager::setPatternFilePath(const QString &path) { m_customPath["patternfilepath"] = path; emit configChanged(); }

// record section getters and setters
int ConfigManager::userRank() const { return m_record.value("userrank", 1).toInt(); }
void ConfigManager::setUserRank(int rank) { m_record["userrank"] = rank; emit configChanged(); }

int ConfigManager::totalVideoNum() const { return m_record.value("totalvideonum", 0).toInt(); }
void ConfigManager::setTotalVideoNum(int num) { m_record["totalvideonum"] = num; emit configChanged(); }

int ConfigManager::totalGroupNum() const { return m_record.value("totalgroupnum", 0).toInt(); }
void ConfigManager::setTotalGroupNum(int num) { m_record["totalgroupnum"] = num; emit configChanged(); }

double ConfigManager::totalUsingTime() const { return m_record.value("totalusingtime", 0.0).toDouble(); }
void ConfigManager::setTotalUsingTime(double time) { m_record["totalusingtime"] = time; emit configChanged(); }

// 路径相关方法
QString ConfigManager::configFilePath() const { return m_configFilePath; }
QString ConfigManager::defaultPatternPath() const { return m_patternPath; }
QString ConfigManager::defaultFfmpegPath() const {
    return customPermission() ? ffmpegPath() : m_ffmpegPath;
}
QString ConfigManager::defaultFfprobePath() const { return m_ffprobePath; }
QString ConfigManager::logFilePath() const { return m_logPath; }