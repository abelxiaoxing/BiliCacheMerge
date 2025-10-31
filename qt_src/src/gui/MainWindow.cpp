#include "MainWindow.h"
#include "DropLineEdit.h"
#include "core/ConfigManager.h"
#include "core/MergeWorker.h"
#include "utils/Logger.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QStatusBar>
#include <QGroupBox>
#include <QSplitter>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_menuBar(nullptr)
    , m_mergeWorker(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_isMerging(false)
    , m_isPaused(false)
{
    // 设置窗口属性
    setWindowTitle(QTBILIMERGE_NAME);
    setMinimumSize(MAIN_WINDOW_MIN_WIDTH, MAIN_WINDOW_MIN_HEIGHT);
    resize(MAIN_WINDOW_DEFAULT_WIDTH, MAIN_WINDOW_DEFAULT_HEIGHT);

    // 启用拖拽
    setAcceptDrops(true);

    // 初始化UI
    setupUI();
    setupMenuBar();
    setupStatusBar();
    connectSignals();
    loadSettings();

    // 设置更新定时器
    m_updateTimer->setInterval(100); // 100ms更新一次
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateUI);
    m_updateTimer->start();

    Logger::info("主窗口初始化完成");
}

MainWindow::~MainWindow()
{
    saveSettings();
    Logger::info("主窗口销毁完成");
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    m_mainLayout = new QVBoxLayout(centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建各个组件
    auto *headerWidget = createHeaderWidget();
    auto *controlWidget = createControlWidget();
    auto *logWidget = createLogWidget();

    // 添加到主布局
    m_mainLayout->addWidget(headerWidget);
    m_mainLayout->addWidget(controlWidget);
    m_mainLayout->addWidget(logWidget, 1); // 日志区域可伸缩

    // 设置样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QPushButton {
            background-color: #fb7299;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #fc8db3;
        }
        QPushButton:pressed {
            background-color: #e85d8a;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        QTextEdit {
            border: 1px solid #cccccc;
            border-radius: 6px;
            background-color: white;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
    )");
}

void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();

    // 选项菜单
    m_menuOptions = m_menuBar->addMenu("选项(&O)");

    m_actionStart = new QAction("开始合并(&S)", this);
    m_actionStart->setShortcut(QKeySequence("F1"));
    connect(m_actionStart, &QAction::triggered, this, &MainWindow::onStartMerge);
    m_menuOptions->addAction(m_actionStart);

    m_actionPause = new QAction("暂停合并(&P)", this);
    m_actionPause->setShortcut(QKeySequence("F2"));
    m_actionPause->setEnabled(false);
    connect(m_actionPause, &QAction::triggered, this, &MainWindow::onPauseMerge);
    m_menuOptions->addAction(m_actionPause);

    m_actionResume = new QAction("继续合并(&R)", this);
    m_actionResume->setShortcut(QKeySequence("F3"));
    m_actionResume->setEnabled(false);
    connect(m_actionResume, &QAction::triggered, this, &MainWindow::onResumeMerge);
    m_menuOptions->addAction(m_actionResume);

    m_menuOptions->addSeparator();

    m_actionSettings = new QAction("设置(&S)...", this);
    m_actionSettings->setShortcut(QKeySequence("F5"));
    connect(m_actionSettings, &QAction::triggered, this, &MainWindow::onSettings);
    m_menuOptions->addAction(m_actionSettings);

    m_menuOptions->addSeparator();

    m_actionQuit = new QAction("退出(&Q)", this);
    m_actionQuit->setShortcut(QKeySequence("F8"));
    connect(m_actionQuit, &QAction::triggered, this, &MainWindow::onQuit);
    m_menuOptions->addAction(m_actionQuit);

    // 工具菜单
    m_menuTools = m_menuBar->addMenu("工具(&T)");

    m_actionPatternBuilder = new QAction("搜索模式创建向导", this);
    connect(m_actionPatternBuilder, &QAction::triggered, this, &MainWindow::onPatternBuilder);
    m_menuTools->addAction(m_actionPatternBuilder);

    m_menuTools->addSeparator();

    m_actionJsonCheck = new QAction("json修复(不可逆，慎用)", this);
    connect(m_actionJsonCheck, &QAction::triggered, this, &MainWindow::onJsonCheck);
    m_menuTools->addAction(m_actionJsonCheck);

    // 帮助菜单
    m_menuHelp = m_menuBar->addMenu("帮助(&H)");

    m_actionTutorial = new QAction("使用说明", this);
    connect(m_actionTutorial, &QAction::triggered, this, &MainWindow::onShowTutorial);
    m_menuHelp->addAction(m_actionTutorial);

    m_menuHelp->addSeparator();

    m_actionFAQ = new QAction("相关问题", this);
    connect(m_actionFAQ, &QAction::triggered, this, &MainWindow::onShowFAQ);
    m_menuHelp->addAction(m_actionFAQ);

    m_actionFeedback = new QAction("反馈", this);
    connect(m_actionFeedback, &QAction::triggered, this, &MainWindow::onShowFeedback);
    m_menuHelp->addAction(m_actionFeedback);

    // 关于菜单
    m_menuAbout = m_menuBar->addMenu("关于(&A)");

    m_actionAbout = new QAction("软件信息", this);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::onShowAbout);
    m_menuAbout->addAction(m_actionAbout);

    m_menuAbout->addSeparator();

    m_actionLog = new QAction("日志", this);
    connect(m_actionLog, &QAction::triggered, this, &MainWindow::onShowLog);
    m_menuAbout->addAction(m_actionLog);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("就绪");
    statusBar()->addWidget(m_statusLabel, 1);

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(PROGRESS_MIN, PROGRESS_MAX);
    m_progressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_progressBar);

    m_progressLabel = new QLabel();
    statusBar()->addPermanentWidget(m_progressLabel);
}

QWidget* MainWindow::createHeaderWidget()
{
    auto *headerWidget = new QWidget();
    auto *layout = new QHBoxLayout(headerWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    // 应用标题
    auto *titleLabel = new QLabel(QTBILIMERGE_NAME);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #fb7299;");
    layout->addWidget(titleLabel);

    layout->addStretch();

    return headerWidget;
}

QWidget* MainWindow::createControlWidget()
{
    auto *controlWidget = new QWidget();
    auto *layout = new QVBoxLayout(controlWidget);
    layout->setSpacing(15);

    // 文件选择组
    auto *fileGroup = createFileSelectionGroup();
    layout->addWidget(fileGroup);

    // 模式选择和操作组
    auto *modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(15);

    auto *modeGroup = createModeSelectionGroup();
    auto *actionGroup = createActionGroup();

    modeLayout->addWidget(modeGroup);
    modeLayout->addWidget(actionGroup, 1); // 操作组可伸缩

    layout->addLayout(modeLayout);

    return controlWidget;
}

QGroupBox* MainWindow::createFileSelectionGroup()
{
    auto *group = new QGroupBox("选择路径");
    auto *layout = new QVBoxLayout(group);

    auto *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(10);

    // 路径输入框
    m_pathEdit = new DropLineEdit();
    m_pathEdit->setPlaceholderText("单击或拖拽文件夹到此处，不填则搜索同目录下的待合并文件");
    m_pathEdit->setReadOnly(true);
    inputLayout->addWidget(m_pathEdit);

    // 浏览按钮
    m_browseButton = new QPushButton("浏览...");
    m_browseButton->setFixedWidth(80);
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::onSelectDirectory);
    inputLayout->addWidget(m_browseButton);

    layout->addLayout(inputLayout);
    return group;
}

QGroupBox* MainWindow::createModeSelectionGroup()
{
    auto *group = new QGroupBox("模式");
    auto *layout = new QVBoxLayout(group);

    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem("bilibili", static_cast<int>(BILIBILI_MODE));
    m_modeComboBox->addItem("通用", static_cast<int>(UNIVERSAL_MODE));
    m_modeComboBox->setCurrentIndex(0);
    connect(m_modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMergeModeChanged);

    layout->addWidget(m_modeComboBox);
    return group;
}

QGroupBox* MainWindow::createActionGroup()
{
    auto *group = new QGroupBox("操作");
    auto *layout = new QVBoxLayout(group);

    m_mergeButton = new QPushButton("开始合并");
    m_mergeButton->setMinimumHeight(50);
    m_mergeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #fb7299;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #fc8db3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e85d8a;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
    );
    connect(m_mergeButton, &QPushButton::clicked, this, &MainWindow::onStartMerge);

    layout->addWidget(m_mergeButton);
    return group;
}

QWidget* MainWindow::createLogWidget()
{
    auto *logGroup = new QGroupBox("处理日志");
    auto *layout = new QVBoxLayout(logGroup);

    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    // QTextEdit在Qt6中已移除setMaximumBlockCount，使用document()代替
    m_logTextEdit->document()->setMaximumBlockCount(1000);

    layout->addWidget(m_logTextEdit);
    return logGroup;
}

void MainWindow::connectSignals()
{
    // 工作线程信号连接
    if (m_mergeWorker) {
        connect(m_mergeWorker.get(), &MergeWorker::started,
                this, &MainWindow::onMergeStarted);
        connect(m_mergeWorker.get(), &MergeWorker::finished,
                this, &MainWindow::onMergeFinished);
        connect(m_mergeWorker.get(), &MergeWorker::progressUpdated,
                this, &MainWindow::onMergeProgressUpdated);
        connect(m_mergeWorker.get(), &MergeWorker::errorOccurred,
                this, &MainWindow::onMergeErrorOccurred);
    }
}

void MainWindow::loadSettings()
{
    auto &config = ConfigManager::getInstance();

    // 加载最后使用的目录
    m_currentDirectory = config.getLastDirectory();
    if (!m_currentDirectory.isEmpty()) {
        m_pathEdit->setText(m_currentDirectory);
    }

    // 加载最后使用的模式
    m_lastPattern = config.getLastPattern();
    // TODO: 设置模式选择框
}

void MainWindow::saveSettings()
{
    auto &config = ConfigManager::getInstance();

    config.setLastDirectory(m_currentDirectory);
    // TODO: 保存其他设置
}

// 槽函数实现
void MainWindow::onStartMerge()
{
    if (m_isMerging) {
        return;
    }

    QString directory = m_pathEdit->text().trimmed();
    if (directory.isEmpty()) {
        directory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    if (!QDir(directory).exists()) {
        QMessageBox::warning(this, "错误", "选择的目录不存在！");
        return;
    }

    m_currentDirectory = directory;

    // 创建并启动工作线程
    m_mergeWorker = std::make_unique<MergeWorker>();
    connectSignals();

    auto mode = static_cast<MergeMode>(m_modeComboBox->currentData().toInt());
    m_mergeWorker->startMerge(directory, mode);
}

void MainWindow::onPauseMerge()
{
    if (m_mergeWorker && m_isMerging && !m_isPaused) {
        m_isPaused = true;
        m_mergeWorker->pause();

        m_mergeButton->setText("继续合并");
        m_actionPause->setEnabled(false);
        m_actionResume->setEnabled(true);

        m_statusLabel->setText("已暂停");
        Logger::info("用户暂停合并操作");
    }
}

void MainWindow::onResumeMerge()
{
    if (m_mergeWorker && m_isMerging && m_isPaused) {
        m_isPaused = false;
        m_mergeWorker->resume();

        m_mergeButton->setText("暂停合并");
        m_actionPause->setEnabled(true);
        m_actionResume->setEnabled(false);

        m_statusLabel->setText("合并中...");
        Logger::info("用户继续合并操作");
    }
}

void MainWindow::onSelectDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(
        this, "浏览文件夹", m_currentDirectory);

    if (!directory.isEmpty()) {
        m_pathEdit->setText(directory);
        m_currentDirectory = directory;
    }
}

void MainWindow::onMergeModeChanged()
{
    // TODO: 根据模式更新UI状态
    Logger::info(QString("合并模式已更改为: %1").arg(m_modeComboBox->currentText()));
}

void MainWindow::onMergeStarted()
{
    m_isMerging = true;
    m_isPaused = false;

    // 更新UI状态
    m_mergeButton->setText("暂停合并");
    m_mergeButton->setEnabled(true);
    m_actionStart->setEnabled(false);
    m_actionPause->setEnabled(true);
    m_actionResume->setEnabled(false);

    // 显示进度条
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_progressLabel->setText("0%");

    m_statusLabel->setText("合并中...");
    m_logTextEdit->append("=== 开始合并 ===");

    Logger::info("合并操作已开始");
}

void MainWindow::onMergeFinished()
{
    m_isMerging = false;
    m_isPaused = false;

    // 更新UI状态
    m_mergeButton->setText("开始合并");
    m_actionStart->setEnabled(true);
    m_actionPause->setEnabled(false);
    m_actionResume->setEnabled(false);

    // 隐藏进度条
    m_progressBar->setVisible(false);
    m_progressLabel->clear();

    m_statusLabel->setText("合并完成");
    m_logTextEdit->append("=== 合并完成 ===");

    Logger::info("合并操作已完成");
}

void MainWindow::onMergeProgressUpdated(int progress, const QString &message)
{
    // 更新进度条
    m_progressBar->setValue(progress);
    m_progressLabel->setText(QString("%1%").arg(progress));

    // 更新状态栏
    m_statusLabel->setText(message);

    // 添加日志
    if (!message.isEmpty()) {
        m_logTextEdit->append(message);
    }
}

void MainWindow::onMergeErrorOccurred(const QString &error)
{
    m_logTextEdit->append(QString("错误: %1").arg(error));
    Logger::error(QString("合并错误: %1").arg(error));

    QMessageBox::critical(this, "合并错误", error);
}

void MainWindow::updateUI()
{
    // 定期更新UI状态
    if (m_isMerging && m_mergeWorker) {
        // 可以在这里更新一些实时状态
    }
}

void MainWindow::updateProgress()
{
    // 更新进度显示
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isMerging) {
        auto ret = QMessageBox::question(
            this, "确认退出",
            "合并操作正在进行中，确定要退出吗？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }

        // 停止合并工作
        if (m_mergeWorker) {
            m_mergeWorker->stop();
        }
    }

    saveSettings();
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString path = urls.first().toLocalFile();
            QFileInfo fileInfo(path);

            if (fileInfo.isDir()) {
                m_pathEdit->setText(path);
                m_currentDirectory = path;
            } else {
                m_pathEdit->setText(fileInfo.absolutePath());
                m_currentDirectory = fileInfo.absolutePath();
            }
        }
    }
}

// 其他槽函数的简单实现
void MainWindow::onSettings() { /* TODO: 实现设置对话框 */ }
void MainWindow::onPatternBuilder() { /* TODO: 实现模式创建向导 */ }
void MainWindow::onJsonCheck() { /* TODO: 实现JSON修复 */ }
void MainWindow::onShowTutorial() { /* TODO: 显示使用说明 */ }
void MainWindow::onShowFAQ() { /* TODO: 显示常见问题 */ }
void MainWindow::onShowFeedback() { /* TODO: 显示反馈 */ }
void MainWindow::onShowAbout() { /* TODO: 显示关于信息 */ }
void MainWindow::onShowLog() { /* TODO: 显示日志文件 */ }
void MainWindow::onQuit() { close(); }