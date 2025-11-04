#include "PatternManager.h"
#include "core/ConfigManager.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonParseError>
#include <QDebug>

PatternManager::PatternManager(ConfigManager *configManager, QObject *parent)
    : QObject(parent)
    , m_configManager(configManager)
{
    initializePatternPath();
}

PatternManager::~PatternManager()
{
}

void PatternManager::initializePatternPath()
{
    if (m_configManager && m_configManager->customPermission()) {
        m_patternPath = m_configManager->patternFilePath();
    } else {
        m_patternPath = m_configManager->defaultPatternPath();
    }
}

bool PatternManager::loadAllPatterns()
{
    m_patterns.clear();

    QDir patternDir(m_patternPath);
    if (!patternDir.exists()) {
        emit patternLoadError(tr("模式目录不存在: %1").arg(m_patternPath));
        return false;
    }

    QStringList patternFiles = patternDir.entryList(QStringList() << "*.pat", QDir::Files);
    if (patternFiles.isEmpty()) {
        emit patternLoadError(tr("未找到模式文件 (.pat) 在目录: %1").arg(m_patternPath));
        return false;
    }

    bool success = true;
    for (const QString &fileName : patternFiles) {
        QString filePath = patternDir.absoluteFilePath(fileName);
        if (!loadPatternFile(filePath)) {
            success = false;
        }
    }

    if (success && !m_patterns.isEmpty()) {
        emit patternsLoaded();
    }

    return success;
}

bool PatternManager::loadPatternByName(const QString &name)
{
    QString filePath = getPatternFilePath(name);
    if (!QFile::exists(filePath)) {
        emit patternLoadError(tr("模式文件不存在: %1").arg(filePath));
        return false;
    }

    return loadPatternFile(filePath);
}

bool PatternManager::loadPatternFile(const QString &filePath)
{
    QVariantMap pattern = parseJsonFile(filePath);
    if (pattern.isEmpty()) {
        return false;
    }

    if (!pattern.contains("name")) {
        emit patternLoadError(tr("模式文件缺少'name'字段: %1").arg(filePath));
        return false;
    }

    QString name = pattern.value("name").toString();
    if (name.isEmpty()) {
        emit patternLoadError(tr("模式文件'name'字段为空: %1").arg(filePath));
        return false;
    }

    if (!validatePattern(pattern)) {
        emit patternLoadError(tr("模式文件验证失败: %1").arg(filePath));
        return false;
    }

    m_patterns[name] = pattern;
    return true;
}

QVariantMap PatternManager::parseJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit patternLoadError(tr("无法打开模式文件: %1").arg(filePath));
        return QVariantMap();
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit patternLoadError(tr("JSON解析错误 (%1:%2): %3")
                             .arg(filePath)
                             .arg(parseError.offset)
                             .arg(parseError.errorString()));
        return QVariantMap();
    }

    if (!doc.isObject()) {
        emit patternLoadError(tr("模式文件必须是JSON对象: %1").arg(filePath));
        return QVariantMap();
    }

    QJsonObject jsonObj = doc.object();
    QVariantMap result;

    // 转换QJsonObject到QVariantMap，使用递归方法
    result = convertJsonObjectToVariantMap(jsonObj);

    return result;
}

bool PatternManager::validatePattern(const QVariantMap &pattern) const
{
    // 检查必需字段
    if (!pattern.contains("name") || pattern.value("name").toString().isEmpty()) {
        return false;
    }

    if (!pattern.contains("search")) {
        return false;
    }

    QVariant searchVariant = pattern.value("search");
    if (!searchVariant.canConvert<QVariantMap>()) {
        return false;
    }

    QVariantMap search = searchVariant.toMap();
    if (!search.contains("name") || search.value("name").toString().isEmpty()) {
        return false;
    }

    if (!search.contains("tree")) {
        return false;
    }

    QVariant treeVariant = search.value("tree");
    if (!treeVariant.canConvert<QVariantMap>()) {
        return false;
    }

    // 检查tree字段
    QVariantMap tree = treeVariant.toMap();
    if (!tree.contains("v") || !tree.contains("a") || !tree.contains("e")) {
        return false; // 视频、音频、入口文件是必需的
    }

    return true;
}

QMap<QString, QVariantMap> PatternManager::patterns() const
{
    return m_patterns;
}

QStringList PatternManager::patternNames() const
{
    return m_patterns.keys();
}

QString PatternManager::getPatternFilePath(const QString &name) const
{
    return QDir(m_patternPath).absoluteFilePath(name + ".pat");
}

QString PatternManager::resolvePathTemplate(const QString &templateStr, const QVariantMap &metadata) const
{
    if (templateStr.isEmpty()) {
        return QString();
    }

    QString result = templateStr;

    // 处理%group%和%episode%变量
    result.replace("%group%", metadata.value("group", "").toString());
    result.replace("%episode%", metadata.value("episode", "").toString());
    result.replace("%type_tag%", metadata.value("type_tag", "").toString());

    // 处理其他JSON字段变量
    for (auto it = metadata.begin(); it != metadata.end(); ++it) {
        QString key = "%" + it.key() + "%";
        QString value = it.value().toString();
        result.replace(key, value);
    }

    return result;
}

QVariantMap PatternManager::parseMetadata(const QString &entryFilePath, const QVariantMap &parseRules) const
{
    QVariantMap metadata;

    // 读取entry.json文件
    QFile entryFile(entryFilePath);
    if (!entryFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return metadata;
    }

    QByteArray jsonData = entryFile.readAll();
    entryFile.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
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

QVariant PatternManager::getValueFromJsonPath(const QJsonObject &jsonObj, const QString &path) const
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

QVariantMap PatternManager::convertJsonObjectToVariantMap(const QJsonObject &jsonObj) const
{
    QVariantMap result;

    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();

        if (value.isObject()) {
            // 递归处理嵌套对象
            result[key] = convertJsonObjectToVariantMap(value.toObject());
        } else if (value.isArray()) {
            // 处理数组
            QJsonArray array = value.toArray();
            QVariantList variantList;
            for (const QJsonValue &arrayValue : array) {
                if (arrayValue.isObject()) {
                    variantList.append(convertJsonObjectToVariantMap(arrayValue.toObject()));
                } else {
                    variantList.append(arrayValue.toVariant());
                }
            }
            result[key] = variantList;
        } else {
            // 基本类型直接转换
            result[key] = value.toVariant();
        }
    }

    return result;
}