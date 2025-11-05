#ifndef PATTERNMANAGER_H
#define PATTERNMANAGER_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class ConfigManager;

/**
 * @brief 模式管理器类
 * 负责加载、解析和管理.pat模式文件
 * 对应Python版本的patternLoad()和patternCheck()方法
 *
 * 模式文件结构:
 * {
 *   "name": "模式名称",
 *   "search": {
 *     "has_group": 0/1,
 *     "name": "入口文件名",
 *     "tree": {
 *       "v": "视频路径模板",
 *       "a": "音频路径模板",
 *       "d": "弹幕路径模板",
 *       "e": "入口文件路径模板",
 *       "c": "封面路径模板"
 *     }
 *   },
 *   "parse": {
 *     "aid": "视频ID字段路径",
 *     "bid": "BV号字段路径",
 *     "cid": "分集ID字段路径",
 *     "title": "标题字段路径",
 *     "part_title": "分集标题字段路径",
 *     "part_id": "分集编号字段路径",
 *     "part_num": "总分集数字段路径",
 *     "cover_url": "封面URL字段路径",
 *     "sid": "季度ID字段路径(可选)"
 *   }
 * }
 */
class PatternManager : public QObject
{
    Q_OBJECT

public:
    explicit PatternManager(ConfigManager *configManager, QObject *parent = nullptr);
    ~PatternManager();

    // 模式加载
    bool loadAllPatterns();
    bool loadPatternByName(const QString &name);
    QMap<QString, QVariantMap> patterns() const;
    QStringList patternNames() const;

    // 模式验证
    bool validatePattern(const QVariantMap &pattern) const;
    QString getPatternFilePath(const QString &name) const;

    // 路径模板处理
    QString resolvePathTemplate(const QString &templateStr, const QVariantMap &metadata) const;
    QVariantMap parseMetadata(const QString &entryFilePath, const QVariantMap &parseRules) const;

signals:
    void patternsLoaded();
    void patternLoadError(const QString &error);

private:
    void initializePatternPath();
    bool loadPatternFile(const QString &filePath);
    QVariantMap parseJsonFile(const QString &filePath);
    QVariant getValueFromJsonPath(const QJsonObject &jsonObj, const QString &path) const;
    QVariantMap convertJsonObjectToVariantMap(const QJsonObject &jsonObj) const;

    ConfigManager* m_configManager;
    QString m_patternPath;
    QMap<QString, QVariantMap> m_patterns;
};

#endif // PATTERNMANAGER_H