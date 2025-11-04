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
{
    // 设置窗口属性
    setWindowTitle(tr("Qt B站缓存合并工具"));
    setMinimumSize(500, 600);
    resize(500, 600);

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
    dirPathLineEdit->setText(path);
    appendLog(tr("[INFO] 设置目录路径: %1").arg(path));

    // 如果设置了路径，自动开始扫描
    QTimer::singleShot(1000, this, &MainWindow::onStartClicked);
}

void MainWindow::createCentralWidget()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建目录选择器
    dirPathLineEdit = new QLineEdit(this);
    dirPathLineEdit->setPlaceholderText(tr("请选择缓存目录..."));

    dirPathButton = new QPushButton(tr("浏览..."), this);

    // 创建日志输出框
    loggerTextEdit = new QTextEdit(this);
    loggerTextEdit->setReadOnly(true);
    loggerTextEdit->setFontFamily("Consolas, Monaco, monospace");

    // 布局设置
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 目录选择布局
    QHBoxLayout* dirLayout = new QHBoxLayout();
    dirLayout->addWidget(dirPathLineEdit);
    dirLayout->addWidget(dirPathButton);
    mainLayout->addLayout(dirLayout);

    // 日志输出区域
    mainLayout->addWidget(loggerTextEdit);

    centralWidget->setLayout(mainLayout);
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
    progressBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 3px; }"
                              "QProgressBar::chunk { background-color: #05B8CC; width: 20px; }");

    // 添加到状态栏
    statusBar->addPermanentWidget(progressBar, 1);
}

void MainWindow::setupConnections()
{
    // 目录选择按钮
    connect(dirPathButton, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择缓存目录"),
                                                      QDir::homePath());
        if (!dir.isEmpty()) {
            dirPathLineEdit->setText(dir);
        }
    });

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
    errorSkipAction->setChecked(false); // 默认不跳过错误
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

// 菜单事件处理实现
void MainWindow::onStartClicked()
{
    appendLog(tr("[INFO] 开始执行合并操作"));

    // 检查必要组件是否已初始化
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

    // 获取用户选择的目录路径
    QString dirPath = dirPathLineEdit->text().trimmed();
    if (dirPath.isEmpty()) {
        appendLog(tr("[ERROR] 请先选择缓存目录"));
        QMessageBox::warning(this, tr("警告"), tr("请先选择缓存目录！"));
        return;
    }

    // 检查目录是否存在
    QDir dir(dirPath);
    if (!dir.exists()) {
        appendLog(tr("[ERROR] 目录不存在: %1").arg(dirPath));
        QMessageBox::warning(this, tr("警告"), tr("指定的目录不存在！"));
        return;
    }

    // 开始扫描文件
    appendLog(tr("[INFO] 开始扫描目录: %1").arg(dirPath));

    // 配置扫描参数
    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = dirPath;
    scanConfig.patternName = QString(); // 使用所有可用模式
    scanConfig.oneDir = false;
    scanConfig.overwrite = false;
    scanConfig.danmuEnabled = true;
    scanConfig.coverEnabled = true;
    scanConfig.subtitleEnabled = false;
    scanConfig.ordered = true;

    // 连接FileScanner信号
    connect(fileScanner, &FileScanner::scanProgress, this,
            [this](int current, int total) {
                progressBar->setMaximum(total);
                progressBar->setValue(current);
            });

    connect(fileScanner, &FileScanner::scanLog, this,
            [this](const QString& message) {
                appendLog(message);
            });

    connect(fileScanner, &FileScanner::scanError, this,
            [this](const QString& error) {
                appendLog(tr("[ERROR] %1").arg(error));
                QMessageBox::critical(this, tr("错误"), tr("扫描失败: %1").arg(error));
            });

    connect(fileScanner, &FileScanner::scanCompleted, this,
            [this](bool success) {
                if (success) {
                    startMergeOperation();
                } else {
                    appendLog(tr("[ERROR] 未找到可合并的文件"));
                    QMessageBox::warning(this, tr("警告"), tr("未找到可合并的文件，请检查目录或文件格式"));
                }
            });

    // 开始扫描
    fileScanner->scan(scanConfig);
}

void MainWindow::onContinueClicked()
{
    appendLog(tr("[INFO] Continue command from user"));
    // TODO: 实现继续逻辑
}

void MainWindow::onPauseClicked()
{
    appendLog(tr("[INFO] Pause command from user"));
    // TODO: 实现暂停逻辑
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
        // 重新初始化FFmpeg路径（如果路径改变了）
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
    // TODO: 实现模式创建向导
}

void MainWindow::onJsonCheckClicked()
{
    appendLog(tr("[WARNING] Json repair function is dangerous!"));
    // TODO: 实现JSON修复功能
}

void MainWindow::onTutorialClicked()
{
    appendLog(tr("[INFO] Tutorial opened"));
    // TODO: 实现使用说明
}

void MainWindow::onQuestionClicked()
{
    appendLog(tr("[INFO] FAQ opened"));
    // TODO: 实现常见问题
}

void MainWindow::onConsultClicked()
{
    appendLog(tr("[INFO] Feedback form opened"));
    // TODO: 实现反馈功能
}

void MainWindow::onAboutClicked()
{
    QMessageBox::about(this, tr("关于"),
                      tr("Qt B站缓存合并工具\n版本: 2.0.0\n\n基于Qt6 + C++开发"));
}

void MainWindow::onLogClicked()
{
    appendLog(tr("[INFO] Log file opened"));
    // TODO: 实现日志查看功能
}

void MainWindow::onQuitClicked()
{
    // 退出逻辑已在setupConnections中连接到qApp->quit()
}

void MainWindow::onErrorSkipToggled(bool checked)
{
    if (checked) {
        appendLog(tr("[INFO] Error skip enabled"));
    } else {
        appendLog(tr("[INFO] Error skip disabled"));
    }
}

void MainWindow::startMergeOperation()
{
    appendLog(tr("[INFO] 开始合并操作"));

    // 获取扫描结果
    QList<FileScanner::VideoGroup> videoGroups = fileScanner->videoGroups();
    if (videoGroups.isEmpty()) {
        appendLog(tr("[ERROR] 没有找到可合并的视频组"));
        return;
    }

    appendLog(tr("[INFO] 找到 %1 个视频组，开始合并").arg(videoGroups.size()));

    // 连接FFmpeg管理器信号
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

    // 开始合并第一个视频组（简化处理）
    // TODO: 实现批量合并处理
    FileScanner::VideoGroup firstGroup = videoGroups.first();
    mergeVideoGroup(firstGroup);
}

void MainWindow::mergeVideoGroup(const FileScanner::VideoGroup& videoGroup)
{
    appendLog(tr("[INFO] 合并视频组: %1").arg(videoGroup.patternName));

    // 获取输出目录（使用输入目录下的merged子目录）
    QString inputDir = dirPathLineEdit->text().trimmed();
    QString outputDir = QDir(inputDir).filePath("merged");
    QDir().mkpath(outputDir);

    // 合并每个视频文件
    for (const FileScanner::VideoFile& videoFile : videoGroup.files) {
        appendLog(tr("[INFO] 合并文件: %1").arg(videoFile.entryPath));

        // 生成输出文件名
        QString title = videoFile.metadata.value("title", "unknown").toString();
        QString outputFile = QDir(outputDir).filePath(QString("%1.mp4").arg(title));

        // 执行合并
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