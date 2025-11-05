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
        emit scanLog(tr("[WARNING] 无法打开文件: %1").arg(entryPath));
        return metadata;
    }

    QByteArray data = file.readAll();
    file.close();

    // 尝试解析JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    // 如果解析失败，尝试检查并修复JSON格式
    if (parseError.error != QJsonParseError::NoError) {
        emit scanLog(tr("[WARNING] JSON解析失败，尝试自动修复: %1").arg(entryPath));
        emit scanLog(tr("错误: %1").arg(parseError.errorString()));

        if (validateAndRepairJson(entryPath, doc)) {
            emit scanLog(tr("[SUCCESS] JSON修复成功"));
        } else {
            emit scanLog(tr("[ERROR] JSON修复失败，跳过此文件"));
            return metadata;
        }
    }

    // 检查是否为对象类型
    if (!doc.isObject()) {
        emit scanLog(tr("[WARNING] JSON不是有效对象格式: %1").arg(entryPath));
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
        ".mp3", ".wav", ".aac", ".flac", ".ogg", ".webm", ".ts", ".m2ts", ".m4s",
        ".blv"  // BLV格式（B站PC客户端）
    };

    for (const QString &ext : validExtensions) {
        if (fileName.endsWith(ext)) {
            // BLV格式需要额外验证
            if (ext == ".blv") {
                return checkBLVFile(filePath);
            }
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

// BLV格式支持相关方法实现

bool FileScanner::isBLVFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName().toLower();
    return fileName.endsWith(".blv");
}

QStringList FileScanner::findBLVSequence(const QDir &dir, const QString &prefix) const
{
    QStringList blvFiles;
    QStringList filters;
    filters << QString("%1*.blv").arg(prefix);

    QStringList entries = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    // 提取序号并排序
    QMap<int, QString> numberedFiles;
    for (const QString &fileName : entries) {
        QRegularExpression regex(QString("%1(\\d+)\\.blv").arg(QRegularExpression::escape(prefix)));
        QRegularExpressionMatch match = regex.match(fileName);
        if (match.hasMatch()) {
            int number = match.captured(1).toInt();
            numberedFiles[number] = fileName;
        }
    }

    // 返回排序后的文件列表
    for (auto it = numberedFiles.begin(); it != numberedFiles.end(); ++it) {
        blvFiles.append(dir.absoluteFilePath(it.value()));
    }

    return blvFiles;
}

bool FileScanner::checkBLVFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 检查BLV文件头（通常是FLV格式的变种）
    QByteArray header = file.read(16);
    file.close();

    // BLV文件通常以特定标识开始
    // 这里简化处理，只要文件存在且非空就认为有效
    return header.size() > 0;
}

void FileScanner::processBLVFiles(VideoFile &videoFile, const QString &directory) const
{
    QDir dir(directory);
    QString entryFileName = QFileInfo(videoFile.entryPath).baseName();

    // 查找所有BLV文件（通常命名为 entry.blv, entry_1.blv 等）
    QStringList blvFiles = findBLVSequence(dir, entryFileName + "_");

    // 如果没有找到带序号的，尝试直接匹配
    if (blvFiles.isEmpty()) {
        QString directBlv = dir.absoluteFilePath(entryFileName + ".blv");
        if (QFile::exists(directBlv)) {
            blvFiles.append(directBlv);
        }
    }

    videoFile.isBlvFormat = !blvFiles.isEmpty();
    videoFile.blvFiles = blvFiles;

    if (videoFile.isBlvFormat) {
        videoFile.blvPath = blvFiles.first(); // 主BLV文件
    }
}

bool FileScanner::validateAndRepairJson(const QString &filePath, QJsonDocument &doc)
{
    QString fixedContent;
    if (checkAndFixJsonFormat(filePath, fixedContent)) {
        QJsonParseError error;
        doc = QJsonDocument::fromJson(fixedContent.toUtf8(), &error);

        if (error.error == QJsonParseError::NoError) {
            return true;
        }
    }

    return false;
}

bool FileScanner::checkAndFixJsonFormat(const QString &filePath, QString &fixedContent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit scanLog(tr("[WARNING] 无法打开JSON文件: %1").arg(filePath));
        return false;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    // 尝试直接解析JSON
    QJsonParseError error;
    QJsonDocument::fromJson(content.toUtf8(), &error);

    // 如果解析成功，直接返回
    if (error.error == QJsonParseError::NoError) {
        fixedContent = content;
        return true;
    }

    emit scanLog(tr("[WARNING] 检测到损坏的JSON文件: %1").arg(filePath));
    emit scanLog(tr("错误类型: %1").arg(error.errorString()));
    emit scanLog(tr("尝试自动修复..."));

    // 尝试修复JSON格式
    fixJsonContent(content);

    // 再次验证
    error = QJsonParseError();
    QJsonDocument::fromJson(content.toUtf8(), &error);

    if (error.error == QJsonParseError::NoError) {
        emit scanLog(tr("[SUCCESS] JSON文件修复成功"));

        // 询问用户是否保存修复后的文件
        // TODO: 可以添加一个选项自动保存
        fixedContent = content;
        return true;
    } else {
        emit scanLog(tr("[ERROR] JSON文件修复失败: %1").arg(error.errorString()));
        return false;
    }
}

void FileScanner::fixJsonContent(QString &content)
{
    // 根据Python版本的jsonCheck实现，主要修复多余的"}"字符

    // 统计左大括号和右大括号
    int leftBraces = content.count('{');
    int rightBraces = content.count('}');

    // 如果右大括号比左大括号多，说明有多余的右括号
    if (rightBraces > leftBraces) {
        emit scanLog(tr("检测到 %1 个多余的右大括号，尝试移除...").arg(rightBraces - leftBraces));

        // 移除多余的右括号
        // 从后往前删除多余的"}"字符
        for (int i = 0; i < rightBraces - leftBraces; ++i) {
            int lastIndex = content.lastIndexOf('}');
            if (lastIndex != -1) {
                content.remove(lastIndex, 1);
            }
        }
    }
    // 如果左大括号比右大括号多，说明缺少右括号
    else if (leftBraces > rightBraces) {
        emit scanLog(tr("检测到缺少 %1 个右大括号，尝试补全...").arg(leftBraces - rightBraces));

        // 在文件末尾补全缺少的右括号
        for (int i = 0; i < leftBraces - rightBraces; ++i) {
            content.append("\n}");
        }
    }

    // 尝试查找更复杂的格式错误
    // 例如：多个连续的右括号 }}
    content.replace("}}", "}");

    // 尝试修复常见的JSON格式问题
    // 移除末尾多余的逗号
    content.replace(QRegularExpression(",\\s*}"), "}");
    content.replace(QRegularExpression(",\\s*]"), "]");

    // 确保文件以正确的字符结尾
    content = content.trimmed();
    if (!content.endsWith('}') && !content.endsWith(']')) {
        content.append("\n}");
    }
}

QString FileScanner::findMatchingBrace(const QString &content, int startPos) const
{
    Q_UNUSED(content);
    Q_UNUSED(startPos);
    // 这个方法为将来扩展预留，当前版本主要处理简单的多余括号问题
    return QString();
}
