#include "Logger.h"
#include "core/ConfigManager.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QMutexLocker>
#include <iostream>

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_currentLogLevel(LOG_INFO)
    , m_consoleOutput(true)
    , m_fileOutput(true)
{
    initialize();
}

Logger::~Logger()
{
    closeLogFile();
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::initialize()
{
    // 确保日志目录存在
    ensureLogDirectory();

    // 设置日志文件路径
    auto &config = ConfigManager::getInstance();
    m_logFilePath = config.getLogFilePath();

    // 尝试打开日志文件
    if (m_fileOutput && !openLogFile()) {
        qWarning() << "Failed to open log file:" << m_logFilePath;
        m_fileOutput = false;
    }

    // 写入启动日志
    log(LOG_INFO, QString("=== %s %s Started ===")
                    .arg(QTBILIMERGE_NAME)
                    .arg(QTBILIMERGE_VERSION),
        "System");

    // 连接信号槽
    connect(this, &Logger::logMessage,
            this, &Logger::handleLogMessage);
}

void Logger::ensureLogDirectory()
{
    QDir logDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
}

bool Logger::openLogFile()
{
    if (m_logFile && m_logFile->isOpen()) {
        return true;
    }

    m_logFile = std::make_unique<QFile>(m_logFilePath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << m_logFile->errorString();
        m_logFile.reset();
        return false;
    }

    m_logStream = std::make_unique<QTextStream>(m_logFile.get());
    // Qt6中QTextStream默认使用UTF-8编码

    return true;
}

void Logger::closeLogFile()
{
    if (m_logStream) {
        m_logStream->flush();
        m_logStream.reset();
    }

    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
        m_logFile.reset();
    }
}

void Logger::log(LogLevel level, const QString &message, const QString &context)
{
    QMutexLocker locker(&m_mutex);

    // 检查日志级别
    if (level > m_currentLogLevel) {
        return;
    }

    // 格式化消息
    QString formattedMessage = formatMessage(level, message, context);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 发送信号
    emit logMessage(level, timestamp, formattedMessage);

    // 输出到控制台
    if (m_consoleOutput) {
        writeToConsole(formattedMessage);
    }

    // 输出到文件
    if (m_fileOutput) {
        writeToFile(formattedMessage);
    }
}

void Logger::info(const QString &message)
{
    getInstance().log(LOG_INFO, message);
}

void Logger::warning(const QString &message)
{
    getInstance().log(LOG_WARNING, message);
}

void Logger::error(const QString &message)
{
    getInstance().log(LOG_ERROR, message);
}

void Logger::debug(const QString &message)
{
    getInstance().log(LOG_DEBUG, message);
}

QString Logger::formatMessage(LogLevel level, const QString &message, const QString &context) const
{
    QString levelStr = levelToString(level);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString formattedMessage = QString("[%1] [%2] ").arg(timestamp, levelStr);

    if (!context.isEmpty()) {
        formattedMessage += QString("[%1] ").arg(context);
    }

    formattedMessage += message;

    return formattedMessage;
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR:   return "ERROR";
        case LOG_DEBUG:   return "DEBUG";
        default:          return "UNKNOWN";
    }
}

void Logger::writeToFile(const QString &message)
{
    if (!m_logStream) {
        if (!openLogFile()) {
            return;
        }
    }

    *m_logStream << message << Qt::endl;

    // 定期刷新缓冲区
    static int flushCounter = 0;
    if (++flushCounter >= 10) {
        flushLog();
        flushCounter = 0;
    }
}

void Logger::writeToConsole(const QString &message)
{
    // 输出到Qt调试控制台
    qDebug() << message;

    // 也可以输出到标准输出/错误流
    std::cout << message.toStdString() << std::endl;
}

void Logger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_currentLogLevel = level;
}

LogLevel Logger::getLogLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentLogLevel;
}

void Logger::setConsoleOutput(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleOutput = enabled;
}

bool Logger::isConsoleOutputEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_consoleOutput;
}

void Logger::setFileOutput(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_fileOutput = enabled;

    if (enabled && !m_logFile) {
        openLogFile();
    } else if (!enabled && m_logFile) {
        closeLogFile();
    }
}

bool Logger::isFileOutputEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_fileOutput;
}

QString Logger::getLogFilePath() const
{
    return m_logFilePath;
}

void Logger::clearLog()
{
    QMutexLocker locker(&m_mutex);

    closeLogFile();

    // 删除日志文件
    if (QFile::exists(m_logFilePath)) {
        QFile::remove(m_logFilePath);
    }

    // 重新打开日志文件
    if (m_fileOutput) {
        openLogFile();
        log(LOG_INFO, "Log file cleared", "System");
    }
}

void Logger::flushLog()
{
    QMutexLocker locker(&m_mutex);

    if (m_logStream) {
        m_logStream->flush();
    }
}

void Logger::flush()
{
    flushLog();
}

void Logger::handleLogMessage(LogLevel level, const QString &timestamp, const QString &message)
{
    // 这里可以添加额外的日志处理逻辑
    // 例如发送到日志监控服务、写入数据库等
    Q_UNUSED(level)
    Q_UNUSED(timestamp)
    Q_UNUSED(message)
}

