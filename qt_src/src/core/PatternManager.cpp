#include "PatternManager.h"

PatternManager::PatternManager(QObject *parent)
    : QObject(parent)
{
}

bool PatternManager::loadPattern(const QString &patternName)
{
    Q_UNUSED(patternName)
    return false;
}

QStringList PatternManager::getAvailablePatterns() const
{
    return QStringList();
}

QJsonObject PatternManager::getCurrentPattern() const
{
    return QJsonObject();
}

bool PatternManager::savePattern(const QString &name, const QJsonObject &pattern)
{
    Q_UNUSED(name)
    Q_UNUSED(pattern)
    return false;
}

bool PatternManager::deletePattern(const QString &name)
{
    Q_UNUSED(name)
    return false;
}

