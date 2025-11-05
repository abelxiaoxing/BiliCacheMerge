#include "LogViewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileSystemWatcher>
#include <QScrollBar>
#include <QTemporaryFile>

LogViewer::LogViewer(QWidget *parent)
    : QDialog(parent)
    , m_logDisplay(nullptr)
    , m_openLogButton(nullptr)
    , m_refreshButton(nullptr)
    , m_clearButton(nullptr)
    , m_exportButton(nullptr)
    , m_closeButton(nullptr)
    , m_autoRefreshCheck(nullptr)
    , m_logPathLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_autoRefreshTimer(new QTimer(this))
{
    setWindowTitle("日志查看器 - B站缓存合并工具");
    setMinimumSize(800, 600);
    resize(800, 600);

    // 居中显示
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    createLogContent();

    // 连接自动刷新定时器
    connect(m_autoRefreshTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentLogPath.isEmpty() && QFile::exists(m_currentLogPath)) {
            loadLogFile(m_currentLogPath);
        }
    });

    // 默认加载最近的日志
    QString defaultLogPath = getDefaultLogPath();
    if (!defaultLogPath.isEmpty()) {
        loadLogFile(defaultLogPath);
    }
}

LogViewer::~LogViewer()
{
    m_autoRefreshTimer->stop();
}

void LogViewer::createLogContent()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);

    // 标题
    QLabel *titleLabel = new QLabel("日志查看器", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    color: #2196F3;"
        "    padding: 5px 0;"
        "}"
    );
    m_mainLayout->addWidget(titleLabel);

    // 控制栏
    m_controlLayout = new QHBoxLayout();
    m_controlLayout->setSpacing(8);

    m_openLogButton = new QPushButton("打开日志文件", this);
    m_openLogButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
    );

    m_refreshButton = new QPushButton("刷新", this);
    m_clearButton = new QPushButton("清空", this);
    m_exportButton = new QPushButton("导出", this);

    m_autoRefreshCheck = new QCheckBox("自动刷新", this);
    m_autoRefreshCheck->setChecked(false);

    m_controlLayout->addWidget(m_openLogButton);
    m_controlLayout->addWidget(m_refreshButton);
    m_controlLayout->addWidget(m_clearButton);
    m_controlLayout->addWidget(m_exportButton);
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_autoRefreshCheck);

    m_mainLayout->addLayout(m_controlLayout);

    // 状态栏
    m_logPathLabel = new QLabel("未加载日志文件", this);
    m_logPathLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 12px;"
        "    padding: 2px 5px;"
        "}"
    );
    m_logPathLabel->setWordWrap(true);

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "    color: #999999;"
        "    font-size: 11px;"
        "    padding: 2px 5px;"
        "}"
    );

    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(m_logPathLabel, 1);
    statusLayout->addWidget(m_statusLabel, 0);
    m_mainLayout->addLayout(statusLayout);

    // 日志显示区域
    m_logDisplay = new QPlainTextEdit(this);
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setStyleSheet(
        "QPlainTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: #fafafa;"
        "    font-family: 'Consolas', 'Courier New', monospace;"
        "    font-size: 12px;"
        "    padding: 8px;"
        "}"
    );
    m_logDisplay->setPlainText(
        "=== 日志查看器 ===\n"
        "\n"
        "欢迎使用日志查看器！\n"
        "\n"
        "使用方法：\n"
        "1. 点击\"打开日志文件\"选择要查看的日志文件\n"
        "2. 或等待程序自动加载默认日志\n"
        "3. 使用\"刷新\"按钮更新日志内容\n"
        "4. 勾选\"自动刷新\"可自动更新日志\n"
        "5. 使用\"导出\"保存日志到其他位置\n"
        "\n"
        "提示：主程序的日志会实时显示在这里\n"
    );

    m_mainLayout->addWidget(m_logDisplay, 1);

    // 关闭按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setDefault(true);
    m_closeButton->setFixedWidth(100);
    buttonLayout->addWidget(m_closeButton);
    m_mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_openLogButton, &QPushButton::clicked, this, &LogViewer::onOpenLogClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &LogViewer::onRefreshClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &LogViewer::onClearClicked);
    connect(m_exportButton, &QPushButton::clicked, this, &LogViewer::onExportClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &LogViewer::onAutoRefreshToggled);
}

void LogViewer::onOpenLogClicked()
{
    QString logPath = QFileDialog::getOpenFileName(
        this,
        "选择日志文件",
        getDefaultLogPath(),
        "日志文件 (*.log *.txt);;所有文件 (*.*)"
    );

    if (!logPath.isEmpty()) {
        loadLogFile(logPath);
    }
}

void LogViewer::onRefreshClicked()
{
    if (!m_currentLogPath.isEmpty()) {
        loadLogFile(m_currentLogPath);
    }
}

void LogViewer::onClearClicked()
{
    m_logDisplay->clear();
    m_statusLabel->setText("日志已清空");
}

void LogViewer::onExportClicked()
{
    if (m_logDisplay->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "警告", "没有可导出的日志内容！");
        return;
    }

    QString exportPath = QFileDialog::getSaveFileName(
        this,
        "导出日志",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/bilimergelog.txt",
        "文本文件 (*.txt);;所有文件 (*.*)"
    );

    if (!exportPath.isEmpty()) {
        QFile file(exportPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << m_logDisplay->toPlainText();
            file.close();

            QMessageBox::information(
                this,
                "成功",
                QString("日志已导出到：\n%1").arg(exportPath)
            );
        } else {
            QMessageBox::critical(this, "错误", "无法创建导出文件！");
        }
    }
}

void LogViewer::onAutoRefreshToggled(bool checked)
{
    if (checked) {
        m_autoRefreshTimer->start(2000); // 每2秒刷新一次
        m_statusLabel->setText("自动刷新已启用");
    } else {
        m_autoRefreshTimer->stop();
        m_statusLabel->setText("自动刷新已禁用");
    }
}

void LogViewer::loadLogFile(const QString &filePath)
{
    m_currentLogPath = filePath;
    m_logPathLabel->setText(QString("日志文件: %1").arg(filePath));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_logDisplay->setPlainText(QString("错误：无法打开文件 %1").arg(filePath));
        m_statusLabel->setText("文件读取失败");
        return;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    // 保存当前滚动位置
    int previousScroll = m_logDisplay->verticalScrollBar()->value();
    bool atBottom = (previousScroll == m_logDisplay->verticalScrollBar()->maximum());

    // 设置内容
    m_logDisplay->setPlainText(content);

    // 自动滚动到底部（如果之前在底部）
    if (atBottom) {
        m_logDisplay->verticalScrollBar()->setValue(
            m_logDisplay->verticalScrollBar()->maximum()
        );
    } else {
        m_logDisplay->verticalScrollBar()->setValue(previousScroll);
    }

    // 更新状态
    QFileInfo fileInfo(filePath);
    QString sizeStr;
    if (fileInfo.size() < 1024) {
        sizeStr = QString::number(fileInfo.size()) + " B";
    } else if (fileInfo.size() < 1024 * 1024) {
        sizeStr = QString::number(fileInfo.size() / 1024.0, 'f', 1) + " KB";
    } else {
        sizeStr = QString::number(fileInfo.size() / (1024.0 * 1024.0), 'f', 1) + " MB";
    }

    m_statusLabel->setText(
        QString("大小: %1, 修改时间: %2")
        .arg(sizeStr)
        .arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"))
    );
}

QString LogViewer::getDefaultLogPath()
{
    // 尝试在多个可能的位置查找日志文件
    QStringList possiblePaths;

    // 1. 程序运行目录
    possiblePaths << QDir::currentPath() + "/bilimergelog.txt";
    possiblePaths << QDir::currentPath() + "/log.txt";

    // 2. 用户配置目录
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    possiblePaths << configPath + "/bilimergelog.txt";
    possiblePaths << configPath + "/log.txt";

    // 3. 系统临时目录
    possiblePaths << QDir::tempPath() + "/bilimergelog.txt";

    // 4. 桌面
    possiblePaths << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/bilimergelog.txt";

    // 查找第一个存在的日志文件
    for (const QString &path : possiblePaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString(); // 未找到日志文件
}

bool LogViewer::openWithSystemApp(const QString &filePath)
{
    Q_UNUSED(filePath);
    // 这个函数为将来扩展预留，用于用系统默认程序打开日志文件
    return false;
}
