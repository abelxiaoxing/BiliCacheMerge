#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <memory>

#include "qtbilimerge.h"

/**
 * @brief 日志系统
 *
 * 提供线程安全的日志记录功能
 * 支持多种日志级别和控制台/文件输出
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger& getInstance();

    // 日志记录方法
    static void info(const QString &message);
    static void warning(const QString &message);
    static void error(const QString &message);
    static void debug(const QString &message);

    // 手动日志记录
    void log(LogLevel level, const QString &message, const QString &context = "");

    // 配置方法
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

    void setConsoleOutput(bool enabled);
    bool isConsoleOutputEnabled() const;

    void setFileOutput(bool enabled);
    bool isFileOutputEnabled() const;

    QString getLogFilePath() const;

    // 文件操作
    void clearLog();
    bool openLogFile();
    void closeLogFile();
    void flushLog();

public slots:
    void flush();

signals:
    void logMessage(LogLevel level, const QString &timestamp, const QString &message);
    void logError(const QString &error);

private slots:
    void handleLogMessage(LogLevel level, const QString &timestamp, const QString &message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    // 禁用拷贝构造和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void initialize();
    void ensureLogDirectory();
    QString formatMessage(LogLevel level, const QString &message, const QString &context = "") const;
    QString levelToString(LogLevel level) const;
    void writeToFile(const QString &message);
    void writeToConsole(const QString &message);

    std::unique_ptr<QFile> m_logFile;
    std::unique_ptr<QTextStream> m_logStream;
    QString m_logFilePath;
    LogLevel m_currentLogLevel;
    bool m_consoleOutput;
    bool m_fileOutput;
    mutable QMutex m_mutex;
};

// 便捷宏定义
#define LOG_INFO(msg) Logger::info(msg)
#define LOG_WARNING(msg) Logger::warning(msg)
#define LOG_ERROR(msg) Logger::error(msg)
#define LOG_DEBUG(msg) Logger::debug(msg)

// 带上下文的日志宏
#define LOG_INFO_CTX(msg, ctx) Logger::getInstance().log(LOG_INFO, msg, ctx)
#define LOG_WARNING_CTX(msg, ctx) Logger::getInstance().log(LOG_WARNING, msg, ctx)
#define LOG_ERROR_CTX(msg, ctx) Logger::getInstance().log(LOG_ERROR, msg, ctx)
#define LOG_DEBUG_CTX(msg, ctx) Logger::getInstance().log(LOG_DEBUG, msg, ctx)

#endif // LOGGER_H