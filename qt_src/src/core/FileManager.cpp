#include "FileManager.h"

FileManager::FileManager(QObject *parent)
    : QObject(parent)
{
}

QStringList FileManager::searchVideoFiles(const QString &directory, const QString &pattern)
{
    Q_UNUSED(directory)
    Q_UNUSED(pattern)
    return QStringList();
}

QStringList FileManager::searchAudioFiles(const QString &directory, const QString &pattern)
{
    Q_UNUSED(directory)
    Q_UNUSED(pattern)
    return QStringList();
}

QStringList FileManager::searchDanmakuFiles(const QString &directory, const QString &pattern)
{
    Q_UNUSED(directory)
    Q_UNUSED(pattern)
    return QStringList();
}

bool FileManager::isValidVideoFile(const QString &filePath) const
{
    Q_UNUSED(filePath)
    return false;
}

bool FileManager::isValidAudioFile(const QString &filePath) const
{
    Q_UNUSED(filePath)
    return false;
}

bool FileManager::isValidDanmakuFile(const QString &filePath) const
{
    Q_UNUSED(filePath)
    return false;
}

