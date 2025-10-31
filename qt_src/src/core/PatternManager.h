#ifndef PATTERNMANAGER_H
#define PATTERNMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonObject>

/**
 * @brief 模式管理器
 *
 * 负责管理和加载不同的客户端模式配置
 */
class PatternManager : public QObject
{
    Q_OBJECT

public:
    explicit PatternManager(QObject *parent = nullptr);

    // 模式加载和管理
    bool loadPattern(const QString &patternName);
    QStringList getAvailablePatterns() const;
    QJsonObject getCurrentPattern() const;

    // 模式操作
    bool savePattern(const QString &name, const QJsonObject &pattern);
    bool deletePattern(const QString &name);

signals:
    void patternLoaded(const QString &name);
    void patternLoadFailed(const QString &error);

private:
    QMap<QString, QJsonObject> m_patterns;
    QString m_currentPattern;
};

#endif // PATTERNMANAGER_H