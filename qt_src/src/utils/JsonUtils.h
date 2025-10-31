#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

/**
 * @brief JSON工具类
 *
 * 提供JSON操作的静态方法
 */
class JsonUtils
{
public:
    JsonUtils() = delete;

    // 读取和写入
    static QJsonObject readJsonFile(const QString &filePath);
    static bool writeJsonFile(const QJsonObject &json, const QString &filePath);
    static QJsonArray readJsonArrayFile(const QString &filePath);
    static bool writeJsonArrayFile(const QJsonArray &array, const QString &filePath);

    // 解析和序列化
    static QJsonObject parseJson(const QString &jsonString);
    static QString toJsonString(const QJsonObject &json, bool compact = true);
    static QJsonArray parseJsonArray(const QString &jsonString);
    static QString toJsonString(const QJsonArray &array, bool compact = true);

    // 值获取
    static QString getString(const QJsonObject &json, const QString &key, const QString &defaultValue = "");
    static int getInt(const QJsonObject &json, const QString &key, int defaultValue = 0);
    static double getDouble(const QJsonObject &json, const QString &key, double defaultValue = 0.0);
    static bool getBool(const QJsonObject &json, const QString &key, bool defaultValue = false);
    static QJsonObject getObject(const QJsonObject &json, const QString &key);
    static QJsonArray getArray(const QJsonObject &json, const QString &key);

    // 值设置
    static void setString(QJsonObject &json, const QString &key, const QString &value);
    static void setInt(QJsonObject &json, const QString &key, int value);
    static void setDouble(QJsonObject &json, const QString &key, double value);
    static void setBool(QJsonObject &json, const QString &key, bool value);
    static void setObject(QJsonObject &json, const QString &key, const QJsonObject &value);
    static void setArray(QJsonObject &json, const QString &key, const QJsonArray &value);

    // 路径访问
    static QVariant getValueByPath(const QJsonObject &json, const QString &path, const QVariant &defaultValue = QVariant());
    static bool setValueByPath(QJsonObject &json, const QString &path, const QVariant &value);

    // 验证和修复
    static bool isValidJson(const QString &jsonString);
    static QJsonObject repairJson(const QString &jsonString);
    static bool validateSchema(const QJsonObject &json, const QJsonObject &schema);
};

#endif // JSONUTILS_H