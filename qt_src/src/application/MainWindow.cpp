#include "MainWindow.h"
#include "ConfigDialog.h"
#include "core/ConfigManager.h"
#include "core/FfmpegManager.h"
#include "core/PatternManager.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , configManager(nullptr)
    , ffmpegManager(nullptr)
    , patternManager(nullptr)
    , fileScanner(nullptr)
    , m_isMerging(false)
{
    // 设置窗口属性
    setWindowTitle(tr("Qt B站缓存合并工具"));
    setMinimumSize(900, 700);
    resize(900, 700);

    // 初始化核心组件
    initializeCoreComponents();

    // 创建UI组件
    createCentralWidget();
    createMenuBar();
    createStatusBar();
    setupConnections();

    // 加载配置
    loadConfig();

    // 显示启动信息
    appendLog(tr("=== Qt B站缓存合并工具 2.0.0 Started ==="));
    appendLog(tr("主窗口初始化完成"));

    // 初始化FFmpeg
    if (ffmpegManager && configManager) {
        QString version = ffmpegManager->ffmpegVersion();
        if (!version.isEmpty()) {
            appendLog(tr("FFmpeg版本: %1").arg(version));
        } else {
            appendLog(tr("FFmpeg载入失败！"));
        }
    }

    // 加载模式文件
    if (patternManager) {
        if (patternManager->loadAllPatterns()) {
            QStringList patterns = patternManager->patternNames();
            appendLog(tr("找到 %1 个.pat文件").arg(patterns.count()));
        }
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setDirectoryPath(const QString &path)
{
    dirPathText->setPlainText(path);
    appendLog(tr("[INFO] 设置目录路径: %1").arg(path));

    // 如果设置了路径，自动开始目录解析
    QTimer::singleShot(500, this, &MainWindow::onDirectorySelected);
}

void MainWindow::createCentralWidget()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // === 顶部标题区域 ===
    headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(120);
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 20, 20, 20);
    headerLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel("B站缓存合并工具", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: #2196F3;"
        "    font-size: 32px;"
        "    font-weight: bold;"
        "    padding: 5px 0;"
        "}"
    );

    QLabel* subtitleLabel = new QLabel("专业的B站缓存处理与合并工具", this);
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 16px;"
        "    padding: 2px 0;"
        "}"
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    headerLayout->addStretch();

    // === 目录选择卡片 ===
    directorySelector = new QWidget(this);
    directorySelector->setObjectName("directorySelector");
    directorySelector->setFixedHeight(120);

    // 卡片阴影效果
    QGraphicsDropShadowEffect* cardShadow = new QGraphicsDropShadowEffect(directorySelector);
    cardShadow->setBlurRadius(15);
    cardShadow->setColor(QColor(0, 0, 0, 30));
    cardShadow->setOffset(0, 4);
    directorySelector->setGraphicsEffect(cardShadow);

    directorySelector->setStyleSheet(
        "#directorySelector {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #E0E0E0;"
        "}"
        "#directorySelector:hover {"
        "    border: 2px solid #2196F3;"
        "}"
    );

    QVBoxLayout* dirCardLayout = new QVBoxLayout(directorySelector);
    dirCardLayout->setContentsMargins(20, 15, 20, 15);
    dirCardLayout->setSpacing(10);

    // 标题和图标
    QHBoxLayout* dirHeaderLayout = new QHBoxLayout();
    directoryIcon = new QLabel(this);
    directoryIcon->setFixedSize(32, 32);
    directoryIcon->setScaledContents(true);
    directoryIcon->setStyleSheet(
        "QLabel {"
        "    background-color: #2196F3;"
        "    border-radius: 4px;"
        "}"
    );

    directoryTitle = new QLabel("选择缓存目录", this);
    directoryTitle->setStyleSheet(
        "QLabel {"
        "    color: #333333;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "}"
    );

    dirHeaderLayout->addWidget(directoryIcon);
    dirHeaderLayout->addWidget(directoryTitle);
    dirHeaderLayout->addStretch();

    // 路径显示和浏览按钮
    QHBoxLayout* dirPathLayout = new QHBoxLayout();
    dirPathText = new QTextEdit(this);
    dirPathText->setMaximumHeight(45);
    dirPathText->setPlaceholderText("请选择B站缓存目录...");
    dirPathText->setStyleSheet(
        "QTextEdit {"
        "    background-color: #F5F5F5;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "    color: #333333;"
        "}"
        "QTextEdit:focus {"
        "    border: 2px solid #2196F3;"
        "    background-color: white;"
        "}"
    );

    // 不使用图标，让文字完美居中
    dirPathButton = new QPushButton("浏览", this);
    dirPathButton->setFixedSize(100, 45);
    dirPathButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    font-weight: 600;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0D47A1;"
        "}"
    );

    dirPathLayout->addWidget(dirPathText);
    dirPathLayout->addWidget(dirPathButton);

    dirCardLayout->addLayout(dirHeaderLayout);
    dirCardLayout->addLayout(dirPathLayout);

    // === 状态指示器 ===
    statusIndicator = new QWidget(this);
    statusIndicator->setFixedHeight(50);
    statusIndicator->hide();

    QHBoxLayout* statusLayout = new QHBoxLayout(statusIndicator);
    statusLayout->setContentsMargins(15, 0, 15, 0);
    statusLayout->setAlignment(Qt::AlignCenter);

    statusIcon = new QLabel(this);
    statusIcon->setFixedSize(24, 24);
    statusIcon->setStyleSheet(
        "QLabel {"
        "    background-color: #2196F3;"
        "    border-radius: 4px;"
        "}"
    );

    statusText = new QLabel("请选择缓存目录", this);
    statusText->setStyleSheet(
        "QLabel {"
        "    color: #333333;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
    );

    statusLayout->addWidget(statusIcon);
    statusLayout->addWidget(statusText);
    statusLayout->addStretch();

    statusIndicator->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    border: 2px solid transparent;"
        "}"
    );

    // 添加阴影效果
    QGraphicsDropShadowEffect* statusShadow = new QGraphicsDropShadowEffect(statusIndicator);
    statusShadow->setBlurRadius(8);
    statusShadow->setColor(QColor(0, 0, 0, 30));
    statusShadow->setOffset(0, 2);
    statusIndicator->setGraphicsEffect(statusShadow);

    // === 开始合并按钮 ===
    // 不使用图标，让文字完美居中
    mergeButton = new QPushButton("开始合并", this);
    mergeButton->setFixedSize(220, 60);
    mergeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #42A5F5, stop:1 #2196F3);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-weight: bold;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E88E5, stop:1 #1976D2);"
        "}"
        "QPushButton:pressed {"
        "    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1565C0, stop:1 #0D47A1);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #BDBDBD;"
        "    color: #757575;"
        "}"
    );
    mergeButton->setEnabled(false);

    // 添加阴影效果
    QGraphicsDropShadowEffect* buttonShadow = new QGraphicsDropShadowEffect(mergeButton);
    buttonShadow->setBlurRadius(10);
    buttonShadow->setColor(QColor(33, 150, 243, 100));
    buttonShadow->setOffset(0, 4);
    mergeButton->setGraphicsEffect(buttonShadow);

    // === 日志容器 ===
    logContainer = new QWidget(this);
    QVBoxLayout* logLayout = new QVBoxLayout(logContainer);
    logLayout->setContentsMargins(0, 10, 0, 0);

    QLabel* logTitle = new QLabel("处理日志", this);
    logTitle->setStyleSheet(
        "QLabel {"
        "    color: #333333;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    padding: 5px 0;"
        "}"
    );

    loggerTextEdit = new QTextEdit(this);
    loggerTextEdit->setReadOnly(true);
    loggerTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #FAFAFA;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "    font-family: 'Consolas', 'Monaco', monospace;"
        "    font-size: 12px;"
        "    color: #333333;"
        "    selection-background-color: #2196F3;"
        "}"
    );
    loggerTextEdit->setMinimumHeight(200);

    logLayout->addWidget(logTitle);
    logLayout->addWidget(loggerTextEdit);

    // === 主布局 ===
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(directorySelector);
    mainLayout->addWidget(statusIndicator);
    mainLayout->addWidget(mergeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(logContainer);

    centralWidget->setLayout(mainLayout);

    // 设置窗口样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #F5F5F5;"
        "}"
        "QMainWindow::separator {"
        "    background-color: transparent;"
        "    width: 0;"
        "    height: 0;"
        "}"
    );
}

void MainWindow::createMenuBar()
{
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 选项菜单
    optionMenu = menuBar->addMenu(tr("选项(&O)"));

    continueAction = optionMenu->addAction(tr("继续(&C)\tF1"));
    continueAction->setShortcut(QKeySequence(Qt::Key_F1));

    optionMenu->addSeparator();

    pauseAction = optionMenu->addAction(tr("暂停(&P)\tF2"));
    pauseAction->setShortcut(QKeySequence(Qt::Key_F2));

    optionMenu->addSeparator();

    errorSkipAction = optionMenu->addAction(tr("跳过未知错误(&S)\tF3"));
    errorSkipAction->setCheckable(true);
    errorSkipAction->setChecked(false);
    errorSkipAction->setShortcut(QKeySequence(Qt::Key_F3));

    optionMenu->addSeparator();

    settingsAction = optionMenu->addAction(tr("设置...(&S)\tF5"));
    settingsAction->setShortcut(QKeySequence(Qt::Key_F5));

    optionMenu->addSeparator();

    quitAction = optionMenu->addAction(tr("退出(&X)\tF8"));
    quitAction->setShortcut(QKeySequence(Qt::Key_F8));

    // 工具菜单
    toolsMenu = menuBar->addMenu(tr("工具(&T)"));

    patternBuildAction = toolsMenu->addAction(tr("搜索模式创建向导"));
    toolsMenu->addSeparator();
    jsonCheckAction = toolsMenu->addAction(tr("json修复(不可逆，慎用)"));

    // 帮助菜单
    helpMenu = menuBar->addMenu(tr("帮助(&H)"));

    tutorialAction = helpMenu->addAction(tr("使用说明"));
    helpMenu->addSeparator();
    questionAction = helpMenu->addAction(tr("相关问题"));
    helpMenu->addSeparator();
    consultAction = helpMenu->addAction(tr("反馈"));

    // 关于菜单
    aboutMenu = menuBar->addMenu(tr("关于(&A)"));

    aboutAction = aboutMenu->addAction(tr("软件信息"));
    aboutMenu->addSeparator();
    logAction = aboutMenu->addAction(tr("日志"));
}

void MainWindow::createStatusBar()
{
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // 创建进度条
    progressBar = new QProgressBar(statusBar);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedHeight(20);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 1px solid grey;"
        "    border-radius: 3px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #05B8CC;"
        "    width: 20px;"
        "}"
    );

    // 添加到状态栏
    statusBar->addPermanentWidget(progressBar, 1);
}

void MainWindow::setupConnections()
{
    // 目录选择按钮
    connect(dirPathButton, &QPushButton::clicked, this, &MainWindow::onDirectorySelected);

    // 开始合并按钮
    connect(mergeButton, &QPushButton::clicked, this, &MainWindow::onMergeButtonClicked);

    // 菜单事件连接
    connect(continueAction, &QAction::triggered, this, &MainWindow::onStartClicked);
    connect(pauseAction, &QAction::triggered, this, &MainWindow::onPauseClicked);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(patternBuildAction, &QAction::triggered, this, &MainWindow::onPatternBuildClicked);
    connect(jsonCheckAction, &QAction::triggered, this, &MainWindow::onJsonCheckClicked);
    connect(tutorialAction, &QAction::triggered, this, &MainWindow::onTutorialClicked);
    connect(questionAction, &QAction::triggered, this, &MainWindow::onQuestionClicked);
    connect(consultAction, &QAction::triggered, this, &MainWindow::onConsultClicked);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(logAction, &QAction::triggered, this, &MainWindow::onLogClicked);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitClicked);
    connect(errorSkipAction, &QAction::toggled, this, &MainWindow::onErrorSkipToggled);

    // 退出应用
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // 配置管理器信号
    if (configManager) {
        connect(configManager, &ConfigManager::configChanged, this, [this]() {
            saveConfig();
        });
    }

    // FileScanner信号连接
    if (fileScanner) {
        connect(fileScanner, &FileScanner::scanProgress, this,
                [this](int current, int total) {
                    progressBar->setMaximum(total);
                    progressBar->setValue(current);
                    statusBar()->showMessage(tr("扫描进度: %1/%2").arg(current).arg(total));
                });

        connect(fileScanner, &FileScanner::scanLog, this,
                [this](const QString& message) {
                    appendLog(message);
                });

        connect(fileScanner, &FileScanner::scanError, this,
                [this](const QString& error) {
                    appendLog(tr("[ERROR] %1").arg(error));
                    onDirectoryParseFailed(error);
                });

        // 统一的scanCompleted处理，根据标志位决定操作
        connect(fileScanner, &FileScanner::scanCompleted, this,
                [this](bool success) {
                    progressBar->reset();
                    statusBar()->clearMessage();
                    if (success) {
                        if (m_isMerging) {
                            // 合并状态：开始合并操作
                            m_isMerging = false;
                            startMergeAfterScan();
                        } else {
                            // 目录解析状态：显示成功
                            onDirectoryParseSuccess();
                        }
                    }
                });
    }
}

void MainWindow::initializeCoreComponents()
{
    // 创建配置管理器
    configManager = new ConfigManager(this);

    // 创建FFmpeg管理器
    ffmpegManager = new FfmpegManager(configManager, this);

    // 创建模式管理器
    patternManager = new PatternManager(configManager, this);

    // 创建文件扫描器
    fileScanner = new FileScanner(configManager, patternManager, this);
}

void MainWindow::loadConfig()
{
    if (!configManager) return;

    // 加载配置文件
    configManager->loadConfig();

    // 设置错误跳过选项
    errorSkipAction->setChecked(false);
}

void MainWindow::saveConfig()
{
    if (!configManager) return;

    // 保存配置文件
    configManager->saveConfig();
}

void MainWindow::appendLog(const QString &message)
{
    if (loggerTextEdit) {
        loggerTextEdit->append(message);
        loggerTextEdit->verticalScrollBar()->setValue(
            loggerTextEdit->verticalScrollBar()->maximum());
    }
}

void MainWindow::onStartClicked()
{
    appendLog(tr("[INFO] 开始扫描目录"));

    if (!fileScanner) {
        appendLog(tr("[ERROR] FileScanner未初始化"));
        QMessageBox::critical(this, tr("错误"), tr("内部错误：FileScanner未初始化"));
        return;
    }

    if (!configManager) {
        appendLog(tr("[ERROR] ConfigManager未初始化"));
        QMessageBox::critical(this, tr("错误"), tr("内部错误：ConfigManager未初始化"));
        return;
    }

    if (!ffmpegManager) {
        appendLog(tr("[ERROR] FfmpegManager未初始化"));
        QMessageBox::critical(this, tr("错误"), tr("内部错误：FfmpegManager未初始化"));
        return;
    }

    QString dirPath = dirPathText->toPlainText().trimmed();
    if (dirPath.isEmpty()) {
        appendLog(tr("[ERROR] 请先选择缓存目录"));
        QMessageBox::warning(this, tr("警告"), tr("请先选择缓存目录！"));
        return;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        appendLog(tr("[ERROR] 目录不存在: %1").arg(dirPath));
        QMessageBox::warning(this, tr("警告"), tr("指定的目录不存在！"));
        return;
    }

    appendLog(tr("[INFO] 开始扫描目录: %1").arg(dirPath));

    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = dirPath;
    scanConfig.patternName = QString();
    scanConfig.oneDir = false;
    scanConfig.overwrite = false;
    scanConfig.danmuEnabled = true;
    scanConfig.coverEnabled = true;
    scanConfig.subtitleEnabled = false;
    scanConfig.ordered = true;

    fileScanner->scan(scanConfig);
}

void MainWindow::onContinueClicked()
{
    appendLog(tr("[INFO] Continue command from user"));
}

void MainWindow::onPauseClicked()
{
    appendLog(tr("[INFO] Pause command from user"));
}

void MainWindow::onSettingsClicked()
{
    if (!configManager) {
        appendLog(tr("[ERROR] 配置管理器未初始化"));
        return;
    }

    ConfigDialog dialog(configManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        appendLog(tr("[INFO] 设置已更新"));
        if (ffmpegManager) {
            QString version = ffmpegManager->ffmpegVersion();
            if (!version.isEmpty()) {
                appendLog(tr("FFmpeg版本: %1").arg(version));
            } else {
                appendLog(tr("FFmpeg载入失败！"));
            }
        }
    }
}

void MainWindow::onPatternBuildClicked()
{
    appendLog(tr("[INFO] Pattern build wizard started"));
}

void MainWindow::onJsonCheckClicked()
{
    appendLog(tr("[WARNING] Json repair function is dangerous!"));
}

void MainWindow::onTutorialClicked()
{
    appendLog(tr("[INFO] Tutorial opened"));
}

void MainWindow::onQuestionClicked()
{
    appendLog(tr("[INFO] FAQ opened"));
}

void MainWindow::onConsultClicked()
{
    appendLog(tr("[INFO] Feedback form opened"));
}

void MainWindow::onAboutClicked()
{
    QMessageBox::about(this, tr("关于"),
                      tr("Qt B站缓存合并工具\n版本: 2.0.0\n\n基于Qt6 + C++开发"));
}

void MainWindow::onLogClicked()
{
    appendLog(tr("[INFO] Log file opened"));
}

void MainWindow::onQuitClicked()
{
}

void MainWindow::onErrorSkipToggled(bool checked)
{
    if (checked) {
        appendLog(tr("[INFO] Error skip enabled"));
    } else {
        appendLog(tr("[INFO] Error skip disabled"));
    }
}

void MainWindow::startMergeAfterScan()
{
    appendLog(tr("[INFO] 扫描完成，开始合并操作"));

    QList<FileScanner::VideoGroup> videoGroups = fileScanner->videoGroups();
    if (videoGroups.isEmpty()) {
        appendLog(tr("[ERROR] 没有找到可合并的视频组"));
        QMessageBox::warning(this, tr("警告"), tr("未找到可合并的视频文件"));
        return;
    }

    appendLog(tr("[INFO] 找到 %1 个视频组，开始合并").arg(videoGroups.size()));

    // 连接FFmpeg信号
    connect(ffmpegManager, &FfmpegManager::ffmpegOutput, this,
            [this](const QString& output) {
                appendLog(tr("[FFmpeg] %1").arg(output));
            });

    connect(ffmpegManager, &FfmpegManager::ffmpegError, this,
            [this](const QString& error) {
                appendLog(tr("[FFmpeg Error] %1").arg(error));
            });

    connect(ffmpegManager, &FfmpegManager::progressUpdated, this,
            [this](double progress) {
                progressBar->setValue(static_cast<int>(progress));
            });

    connect(ffmpegManager, &FfmpegManager::ffmpegFinished, this,
            [this](bool success) {
                if (success) {
                    appendLog(tr("[INFO] 合并操作完成"));
                    QMessageBox::information(this, tr("成功"), tr("视频合并完成！"));
                } else {
                    appendLog(tr("[ERROR] 合并操作失败"));
                    QMessageBox::critical(this, tr("错误"), tr("视频合并失败！"));
                }
            });

    // 开始合并第一个视频组
    FileScanner::VideoGroup firstGroup = videoGroups.first();
    mergeVideoGroup(firstGroup);
}

void MainWindow::startMergeOperation()
{
    appendLog(tr("[INFO] 开始合并操作"));

    // 检查必要组件
    if (!fileScanner || !ffmpegManager) {
        appendLog(tr("[ERROR] 核心组件未初始化"));
        QMessageBox::critical(this, tr("错误"), tr("核心组件未初始化"));
        return;
    }

    // 获取并验证目录路径
    QString dirPath = dirPathText->toPlainText().trimmed();
    if (dirPath.isEmpty()) {
        appendLog(tr("[ERROR] 请先选择缓存目录"));
        QMessageBox::warning(this, tr("警告"), tr("请先选择缓存目录！"));
        return;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        appendLog(tr("[ERROR] 目录不存在: %1").arg(dirPath));
        QMessageBox::warning(this, tr("警告"), tr("指定的目录不存在！"));
        return;
    }

    appendLog(tr("[INFO] 开始扫描目录: %1").arg(dirPath));

    // 配置扫描参数
    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = dirPath;
    scanConfig.patternName = QString();
    scanConfig.oneDir = false;
    scanConfig.overwrite = false;
    scanConfig.danmuEnabled = true;
    scanConfig.coverEnabled = true;
    scanConfig.subtitleEnabled = false;
    scanConfig.ordered = true;

    // 连接扫描完成的信号，用于自动开始合并
    connect(fileScanner, &FileScanner::scanCompleted, this,
            [this](bool success) {
                if (success) {
                    // 扫描成功，开始合并
                    QList<FileScanner::VideoGroup> videoGroups = fileScanner->videoGroups();
                    if (videoGroups.isEmpty()) {
                        appendLog(tr("[ERROR] 没有找到可合并的视频组"));
                        QMessageBox::warning(this, tr("警告"), tr("未找到可合并的视频文件"));
                        return;
                    }

                    appendLog(tr("[INFO] 找到 %1 个视频组，开始合并").arg(videoGroups.size()));

                    // 连接FFmpeg信号
                    connect(ffmpegManager, &FfmpegManager::ffmpegOutput, this,
                            [this](const QString& output) {
                                appendLog(tr("[FFmpeg] %1").arg(output));
                            });

                    connect(ffmpegManager, &FfmpegManager::ffmpegError, this,
                            [this](const QString& error) {
                                appendLog(tr("[FFmpeg Error] %1").arg(error));
                            });

                    connect(ffmpegManager, &FfmpegManager::progressUpdated, this,
                            [this](double progress) {
                                progressBar->setValue(static_cast<int>(progress));
                            });

                    connect(ffmpegManager, &FfmpegManager::ffmpegFinished, this,
                            [this](bool success) {
                                if (success) {
                                    appendLog(tr("[INFO] 合并操作完成"));
                                    QMessageBox::information(this, tr("成功"), tr("视频合并完成！"));
                                } else {
                                    appendLog(tr("[ERROR] 合并操作失败"));
                                    QMessageBox::critical(this, tr("错误"), tr("视频合并失败！"));
                                }
                            });

                    // 开始合并第一个视频组
                    FileScanner::VideoGroup firstGroup = videoGroups.first();
                    mergeVideoGroup(firstGroup);
                } else {
                    appendLog(tr("[ERROR] 扫描失败，无法合并"));
                    QMessageBox::critical(this, tr("错误"), tr("扫描失败，请检查目录格式"));
                }
            }, Qt::UniqueConnection);

    // 开始扫描
    fileScanner->scan(scanConfig);
}

void MainWindow::mergeVideoGroup(const FileScanner::VideoGroup& videoGroup)
{
    appendLog(tr("[INFO] 合并视频组: %1").arg(videoGroup.patternName));

    QString inputDir = dirPathText->toPlainText().trimmed();
    QString outputDir = QDir(inputDir).filePath("merged");
    QDir().mkpath(outputDir);

    for (const FileScanner::VideoFile& videoFile : videoGroup.files) {
        appendLog(tr("[INFO] 合并文件: %1").arg(videoFile.entryPath));

        QString title = videoFile.metadata.value("title", "unknown").toString();
        QString outputFile = QDir(outputDir).filePath(QString("%1.mp4").arg(title));

        double progress = 0.0;
        bool success = ffmpegManager->mergeVideoAudio(
            videoFile.videoPath,
            videoFile.audioPath,
            outputFile,
            progress
        );

        if (!success) {
            appendLog(tr("[ERROR] 合并失败: %1").arg(videoFile.entryPath));
            if (!errorSkipAction->isChecked()) {
                return;
            }
        }
    }
}

void MainWindow::onDirectorySelected()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择缓存目录"),
                                                  QDir::homePath());
    if (!dir.isEmpty()) {
        dirPathText->setPlainText(dir);
        appendLog(tr("[INFO] 用户选择目录: %1").arg(dir));

        statusIcon->setStyleSheet(
            "QLabel {"
            "    background-color: #2196F3;"
            "    border-radius: 4px;"
            "}"
        );
        statusText->setText("正在解析目录...");
        statusText->setStyleSheet(
            "QLabel {"
            "    color: #2196F3;"
            "    font-size: 14px;"
            "    font-weight: 500;"
            "}"
        );
        statusIndicator->show();

        // 只做目录扫描，不触发合并
        m_isMerging = false;
        QTimer::singleShot(300, this, [this, dir]() {
            if (fileScanner && configManager) {
                FileScanner::ScanConfig scanConfig;
                scanConfig.searchPath = dir;
                scanConfig.patternName = QString();
                scanConfig.oneDir = false;
                scanConfig.overwrite = false;
                scanConfig.danmuEnabled = true;
                scanConfig.coverEnabled = true;
                scanConfig.subtitleEnabled = false;
                scanConfig.ordered = true;

                fileScanner->scan(scanConfig);
            } else {
                onDirectoryParseFailed("核心组件未初始化");
            }
        });
    }
}

void MainWindow::onDirectoryParseSuccess()
{
    statusIcon->setStyleSheet(
        "QLabel {"
        "    background-color: #4CAF50;"
        "    border-radius: 4px;"
        "}"
    );
    statusText->setText("缓存目录解析成功！发现可合并的视频文件");
    statusText->setStyleSheet(
        "QLabel {"
        "    color: #4CAF50;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
    );
    statusIndicator->show();

    mergeButton->setEnabled(true);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(statusIndicator, "windowOpacity", this);
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start();

    appendLog(tr("[SUCCESS] 目录解析完成，可以开始合并操作"));
}

void MainWindow::onDirectoryParseFailed(const QString &error)
{
    statusIcon->setStyleSheet(
        "QLabel {"
        "    background-color: #F44336;"
        "    border-radius: 4px;"
        "}"
    );
    statusText->setText(tr("缓存目录解析失败: %1").arg(error));
    statusText->setStyleSheet(
        "QLabel {"
        "    color: #F44336;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
    );
    statusIndicator->show();

    mergeButton->setEnabled(false);

    appendLog(tr("[ERROR] 目录解析失败: %1").arg(error));
}

void MainWindow::onMergeButtonClicked()
{
    // 设置合并标志并开始扫描
    m_isMerging = true;
    onStartClicked();
}
