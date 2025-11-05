#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QScrollBar>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include "core/ConfigManager.h"
#include "core/FfmpegManager.h"
#include "core/PatternManager.h"
#include "core/FileScanner.h"

/**
 * @brief Qt版本的BiliCacheMerge主窗口
 * 对应Python版本的mainFrame类
 *
 * 功能特性:
 * - 目录路径选择
 * - 合并模式选择
 * - 实时日志输出
 * - 进度条和状态栏
 * - 菜单栏: 选项、工具、帮助、关于
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setDirectoryPath(const QString &path);

private slots:
    // 菜单事件处理
    void onStartClicked();
    void onContinueClicked();
    void onPauseClicked();
    void onSettingsClicked();
    void onPatternBuildClicked();
    void onJsonCheckClicked();
    void onTutorialClicked();
    void onQuestionClicked();
    void onConsultClicked();
    void onAboutClicked();
    void onLogClicked();
    void onQuitClicked();

    // 错误跳过选项
    void onErrorSkipToggled(bool checked);

    // 拖拽事件处理
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    // 目录操作
    void onDirectorySelected();
    void onDirectoryParseSuccess();
    void onDirectoryParseFailed(const QString &error);
    void onMergeButtonClicked();

    // 合并相关
    void startMergeAfterScan();

private:
    void createMenuBar();
    void createCentralWidget();
    void createStatusBar();
    void setupConnections();
    void initializeCoreComponents();
    void loadConfig();
    void saveConfig();
    void appendLog(const QString &message);
    void startMergeOperation();
    void mergeVideoGroup(const FileScanner::VideoGroup& videoGroup);

    // UI组件
    QWidget* centralWidget;

    // 主要功能区域
    QWidget* headerWidget;           // 顶部标题区域
    QWidget* directorySelector;      // 目录选择卡片
    QLabel* directoryIcon;           // 目录图标
    QLabel* directoryTitle;          // 目录选择标题
    QPushButton* dirPathButton;      // 浏览按钮
    QTextEdit* dirPathText;          // 目录路径显示
    QWidget* statusIndicator;        // 状态指示器容器
    QLabel* statusIcon;              // 状态图标
    QLabel* statusText;              // 状态文本
    QPushButton* mergeButton;        // 开始合并按钮
    QWidget* logContainer;           // 日志容器
    QTextEdit* loggerTextEdit;       // 日志输出框
    QProgressBar* progressBar;       // 进度条（使用内置组件）

    // 状态标志
    bool m_isMerging;                // 是否处于合并状态

    // 核心组件
    ConfigManager* configManager;
    FfmpegManager* ffmpegManager;
    PatternManager* patternManager;
    FileScanner* fileScanner;

    // 菜单项
    QAction* continueAction;
    QAction* pauseAction;
    QAction* errorSkipAction;
    QAction* settingsAction;
    QAction* patternBuildAction;
    QAction* jsonCheckAction;
    QAction* tutorialAction;
    QAction* questionAction;
    QAction* consultAction;
    QAction* aboutAction;
    QAction* logAction;
    QAction* quitAction;

    // 菜单
    QMenu* optionMenu;
    QMenu* toolsMenu;
    QMenu* helpMenu;
    QMenu* aboutMenu;
};

#endif // MAINWINDOW_H