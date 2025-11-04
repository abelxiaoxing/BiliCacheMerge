#include "FileScanner.h"
#include "core/ConfigManager.h"
#include "core/PatternManager.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>

FileScanner::FileScanner(ConfigManager *configManager, PatternManager *patternManager, QObject *parent)
    : QObject(parent)
    , m_configManager(configManager)
    , m_patternManager(patternManager)
    , m_totalFiles(0)
    , m_totalGroups(0)
{
    // 创建非法字符正则表达式（用于文件名清理）
    m_invalidCharsRegex = QRegularExpression("[\\x00-\\x1f\"/:*?\"<>| ]");
}

FileScanner::~FileScanner()
{
}

bool FileScanner::scan(const ScanConfig &config)
{
    m_config = config;
    m_videoGroups.clear();
    m_totalFiles = 0;
    m_totalGroups = 0;

    emit scanLog(tr("开始扫描目录: %1").arg(config.searchPath));

    // 获取要使用的模式
    QMap<QString, QVariantMap> patterns;
    if (!config.patternName.isEmpty()) {
        // 使用指定模式
        if (m_patternManager->loadPatternByName(config.patternName)) {
            patterns = m_patternManager->patterns();
        } else {
            emit scanError(tr("无法加载指定模式: %1").arg(config.patternName));
            return false;
        }
    } else {
        // 使用所有可用模式
        if (!m_patternManager->loadAllPatterns()) {
            emit scanError(tr("无法加载任何模式文件"));
            return false;
        }
        patterns = m_patternManager->patterns();
    }

    // 扫描目录
    bool success = false;
    for (const auto &pattern : patterns) {
        if (scanDirectory(config.searchPath, pattern)) {
            success = true;
        }
    }

    if (success) {
        emit scanLog(tr("扫描完成，找到 %1 组共 %2 个文件").arg(m_totalGroups).arg(m_totalFiles));
        emit scanCompleted(true);
    } else {
        emit scanLog(tr("未找到可合并的文件，请检查路径或文件格式"));
        emit scanCompleted(false);
    }

    return success;
}

QList<FileScanner::VideoGroup> FileScanner::videoGroups() const
{
    return m_videoGroups;
}

int FileScanner::totalFiles() const
{
    return m_totalFiles;
}

int FileScanner::totalGroups() const
{
    return m_totalGroups;
}

bool FileScanner::scanDirectory(const QString &path, const QVariantMap &pattern)
{
    QDir dir(path);
    if (!dir.exists()) {
        emit scanError(tr("目录不存在: %1").arg(path));
        return false;
    }

    // 获取模式信息
    QString patternName = pattern.value("name").toString();
    QVariantMap searchSection = pattern.value("search").toMap();
    bool hasGroup = searchSection.value("has_group").toInt() == 1;
    QString entryName = searchSection.value("name").toString();
    QVariantMap treeSection = searchSection.value("tree").toMap();

    emit scanLog(tr("使用模式 %1 扫描目录: %2").arg(patternName).arg(path));

    // 获取目录中的所有条目
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // 首先检查非分组模式（单个视频）
    if (!hasGroup) {
        for (const QFileInfo &entry : entries) {
            if (entry.isFile() && entry.fileName() == entryName) {
                if (isEntryFile(entry.absoluteFilePath(), pattern)) {
                    // 找到entry文件，创建视频文件组
                    VideoFile videoFile;
                    videoFile.entryPath = entry.absoluteFilePath();

                    // 解析视频和音频路径
                    QString videoTemplate = treeSection.value("v").toString();
                    QString audioTemplate = treeSection.value("a").toString();
                    videoFile.videoPath = resolvePathTemplate(videoTemplate, entry.absoluteFilePath(), pattern);
                    videoFile.audioPath = resolvePathTemplate(audioTemplate, entry.absoluteFilePath(), pattern);

                    // 解析弹幕路径
                    QString danmuTemplate = treeSection.value("d").toString();
                    if (!danmuTemplate.isEmpty() && danmuTemplate != "null") {
                        videoFile.danmuPath = resolvePathTemplate(danmuTemplate, entry.absoluteFilePath(), pattern);
                    }

                    // 提取元数据
                    videoFile.metadata = extractMetadata(entry.absoluteFilePath(), pattern.value("parse").toMap());

                    // 验证媒体文件对是否有效
                    if (!hasValidMediaPair(videoFile)) {
                        emit scanLog(tr("跳过无效的媒体文件对: %1").arg(videoFile.entryPath));
                        continue;
                    }

                    // 创建视频组
                    VideoGroup group;
                    group.patternName = patternName;
                    group.groupEntryPath = QString(); // 非分组模式没有组entry
                    group.files.append(videoFile);
                    group.groupMetadata = videoFile.metadata;

                    m_videoGroups.append(group);
                    m_totalFiles++;
                    m_totalGroups++;

                    emit scanLog(tr("找到视频文件: %1").arg(videoFile.entryPath));
                    return true;
                }
            }
        }
    }

    // 检查分组模式
    if (hasGroup) {
        // 查找组entry文件（通常在根目录）
        QString groupEntryPath;
        for (const QFileInfo &entry : entries) {
            if (entry.isFile()) {
                QString checkName = entryName;
                if (checkName.startsWith('.')) {
                    checkName = dir.dirName() + checkName;
                }
                if (entry.fileName() == checkName) {
                    if (isEntryFile(entry.absoluteFilePath(), pattern)) {
                        groupEntryPath = entry.absoluteFilePath();
                        break;
                    }
                }
            }
        }

        if (!groupEntryPath.isEmpty()) {
            // 找到组entry，现在扫描子目录
            VideoGroup group;
            group.patternName = patternName;
            group.groupEntryPath = groupEntryPath;
            group.groupMetadata = extractMetadata(groupEntryPath, pattern.value("parse").toMap());

            // 解析封面路径
            QString coverTemplate = treeSection.value("c").toString();
            if (!coverTemplate.isEmpty() && coverTemplate != "null") {
                group.coverPath = resolvePathTemplate(coverTemplate, groupEntryPath, pattern);
            }

            // 扫描子目录
            for (const QFileInfo &entry : entries) {
                if (entry.isDir()) {
                    QString subDirPath = entry.absoluteFilePath();
                    QString videoEntryTemplate = treeSection.value("e").toString();
                    QString videoEntryPath = resolvePathTemplate(videoEntryTemplate, subDirPath, pattern);

                    if (QFile::exists(videoEntryPath)) {
                        VideoFile videoFile;
                        videoFile.entryPath = videoEntryPath;
                        videoFile.videoPath = subDirPath + "/" + treeSection.value("v").toString();
                        videoFile.audioPath = subDirPath + "/" + treeSection.value("a").toString();

                        QString danmuTemplate = treeSection.value("d").toString();
                        if (!danmuTemplate.isEmpty() && danmuTemplate != "null") {
                            videoFile.danmuPath = resolvePathTemplate(danmuTemplate, videoEntryPath, pattern);
                        }

                        videoFile.metadata = extractMetadata(videoEntryPath, pattern.value("parse").toMap());

                        // 验证媒体文件对是否有效
                        if (hasValidMediaPair(videoFile)) {
                            group.files.append(videoFile);
                            m_totalFiles++;
                        } else {
                            emit scanLog(tr("跳过无效的媒体文件对: %1").arg(videoFile.entryPath));
                        }
                    }
                }
            }

            if (!group.files.isEmpty()) {
                m_videoGroups.append(group);
                m_totalGroups++;
                emit scanLog(tr("找到视频组: %1 (包含 %2 个文件)").arg(groupEntryPath).arg(group.files.size()));
                return true;
            }
        }
    }

    // 递归扫描子目录
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            if (scanDirectory(entry.absoluteFilePath(), pattern)) {
                return true;
            }
        }
    }

    return false;
}

bool FileScanner::isEntryFile(const QString &filePath, const QVariantMap &pattern)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject jsonObj = doc.object();
    QVariantMap parseSection = pattern.value("parse").toMap();

    // 检查必要的字段是否存在
    QString key = parseSection.value("sid").toString();
    if (key.isEmpty()) {
        key = parseSection.value("aid").toString();
    }

    if (key.isEmpty()) {
        return false;
    }

    QVariant value = getValueFromJsonPath(jsonObj, key);
    return !value.isNull() && !value.toString().isEmpty();
}

QString FileScanner::resolvePathTemplate(const QString &templateStr, const QString &entryPath, const QVariantMap &pattern)
{
    // pattern参数保留用于未来扩展，当前实现直接从entryPath读取JSON
    Q_UNUSED(pattern);

    if (templateStr.isEmpty() || templateStr == "null") {
        return QString();
    }

    QString result = templateStr;

    // 首先处理路径分隔符（在变量替换之前）
    if (QDir::separator() == '/') {
        result.replace('\\', '/');
    } else {
        result.replace('/', '\\');
    }

    // 获取entry文件的目录
    QFileInfo entryInfo(entryPath);
    QString entryDir = entryInfo.absolutePath();
    QString entryBaseName = entryInfo.baseName();

    // 处理%group%和%episode%变量
    result.replace("%group%", entryDir.split('/').last());
    result.replace("%episode%", entryBaseName);

    // 处理其他JSON字段变量（包括嵌套字段）
    QFile entryFile(entryPath);
    if (entryFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = entryFile.readAll();
        entryFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject jsonObj = doc.object();

            // 预先提取所有可能的变量值
            QMap<QString, QString> variables;

            // 处理根级别字段
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                QString key = it.key();
                QVariant value = it.value().toVariant();
                if (value.isValid()) {
                    variables["%" + key + "%"] = value.toString();
                }
            }

            // 处理常见的嵌套字段（如page_data-cid）
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                if (it.value().isObject()) {
                    QJsonObject subObj = it.value().toObject();
                    for (auto subIt = subObj.begin(); subIt != subObj.end(); ++subIt) {
                        QString nestedKey = it.key() + "-" + subIt.key();
                        QVariant value = subIt.value().toVariant();
                        if (value.isValid()) {
                            variables["%" + nestedKey + "%"] = value.toString();
                        }
                    }
                }
            }

            // 执行变量替换
            for (auto varIt = variables.begin(); varIt != variables.end(); ++varIt) {
                result.replace(varIt.key(), varIt.value());
            }
        }
    }

    // 如果结果是相对路径，需要转换为绝对路径
    if (!QDir::isAbsolutePath(result)) {
        result = entryDir + "/" + result;
    }

    return result;
}

QVariantMap FileScanner::extractMetadata(const QString &entryPath, const QVariantMap &parseRules)
{
    QVariantMap metadata;

    QFile file(entryPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return metadata;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return metadata;
    }

    QJsonObject jsonObj = doc.object();

    // 根据parse规则提取元数据
    for (auto it = parseRules.begin(); it != parseRules.end(); ++it) {
        QString field = it.key();
        QString jsonPath = it.value().toString();

        if (!jsonPath.isEmpty()) {
            QVariant value = getValueFromJsonPath(jsonObj, jsonPath);
            if (!value.isNull()) {
                metadata[field] = value;
            }
        }
    }

    return metadata;
}

QVariant FileScanner::getValueFromJsonPath(const QJsonObject &jsonObj, const QString &path) const
{
    if (path.isEmpty()) {
        return QVariant();
    }

    // 处理多级路径，如 "page_data-cid"
    QStringList parts = path.split('-');
    QJsonValue current = jsonObj;

    for (const QString &part : parts) {
        if (current.isObject()) {
            current = current.toObject().value(part);
        } else {
            return QVariant(); // 路径无效
        }
    }

    return current.toVariant();
}

bool FileScanner::validateFilePath(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile();
}

bool FileScanner::validateMediaFile(const QString &filePath) const
{
    // 首先检查文件是否存在
    if (!validateFilePath(filePath)) {
        return false;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName().toLower();

    // 检查常见的视频/音频文件扩展名
    QStringList validExtensions = {
        ".mp4", ".m4a", ".m4v", ".mov", ".avi", ".mkv", ".flv", ".wmv",
        ".mp3", ".wav", ".aac", ".flac", ".ogg", ".webm", ".ts", ".m2ts", ".m4s"
    };

    for (const QString &ext : validExtensions) {
        if (fileName.endsWith(ext)) {
            return true;
        }
    }

    // 如果没有有效扩展名，尝试检查文件头（magic bytes）
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray header = file.read(12);
    file.close();

    // 检查常见的媒体文件头
    if (header.startsWith("ftyp") ||           // MP4
        (header.size() >= 20 && header.mid(4, 4) == "ftyp") || // MP4 variant
        header.startsWith("RIFF") ||           // WAV/AVI
        header.startsWith("OggS") ||           // OGG
        (header.size() >= 4 && header[0] == '\x1a' && header[1] == '\x45' &&
         header[2] == '\xdf' && header[3] == '\xa3') || // Matroska (MKV/WebM)
        header.startsWith("FLV") ||            // FLV
        header.startsWith("ID3") ||            // MP3 with ID3 tag
        (header.size() >= 2 && header[0] == '\xff' && (header[1] & 0xe0) == 0xe0)) { // MP3 frame
        return true;
    }

    return false;
}

bool FileScanner::hasValidMediaPair(const VideoFile &videoFile) const
{
    // 检查视频文件是否有效
    bool videoValid = validateMediaFile(videoFile.videoPath);
    // 检查音频文件是否有效
    bool audioValid = validateMediaFile(videoFile.audioPath);

    // 至少需要视频或音频文件中的一个有效
    // 在B站缓存中，有时可能只有视频或只有音频
    return videoValid || audioValid;
}