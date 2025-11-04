#ifndef MODERNMAINWINDOW_H
#define MODERNMAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QStackedWidget>
#include <QSplitter>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>

#include "core/ConfigManager.h"
#include "core/FfmpegManager.h"
#include "core/PatternManager.h"
#include "core/FileScanner.h"
#include "utils/IconManager.h"

/**
 * @brief 现代化BiliCacheMerge主窗口
 *
 * 新增功能特性:
 * - 现代化UI设计，支持主题切换
 * - 高分辨率图标支持
 * - 突出的"开始合并"主按钮
 * - 缓存目录自动解析和反馈
 * - 动画效果和视觉反馈
 * - 响应式布局设计
 */
class ModernMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ModernMainWindow(QWidget *parent = nullptr);
    ~ModernMainWindow();
    void setDirectoryPath(const QString &path);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 主工具栏按钮事件
    void onDirectoryButtonClicked();
    void onMergeButtonClicked();
    void onSettingsButtonClicked();
    void onAboutButtonClicked();

    // 侧边栏导航事件
    void onNavigationButtonClicked();

    // 缓存目录解析相关
    void onDirectorySelected(const QString &path);
    void onDirectoryParseResult(bool success, const QString &message);

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
    void onErrorSkipToggled(bool checked);

    // 状态更新
    void updateMergeProgress(int current, int total);
    void updateMergeStatus(const QString &status);

private:
    void setupUI();
    void createToolbar();
    void createNavigationPanel();
    void createMainContentArea();
    void createStatusBar();
    void createMenuBar();
    void setupConnections();
    void initializeCoreComponents();
    void loadConfig();
    void saveConfig();
    void applyModernStyle();
    void appendLog(const QString &message, const QString &type = "info");
    void startMergeOperation();
    void mergeVideoGroup(const FileScanner::VideoGroup& videoGroup);

    // 工具栏创建
    QPushButton* createMainButton(const QString& text, IconManager::IconType icon);
    QPushButton* createSecondaryButton(const QString& text, IconManager::IconType icon);
    QToolButton* createNavigationButton(const QString& text, IconManager::IconType icon);

    // 状态指示器
    void updateDirectoryStatus(bool success, const QString &message);
    void showStatusMessage(const QString &message, const QString &type = "info");

    // 动画效果
    void animateButton(QPushButton *button);
    void showStatusToast(const QString &message, const QString &type);

    // UI组件 - 工具栏
    QFrame* toolbarFrame;
    QPushButton* directoryButton;
    QPushButton* mergeButton;
    QPushButton* settingsButton;
    QPushButton* aboutButton;

    // UI组件 - 侧边栏
    QFrame* navigationPanel;
    QToolButton* navDirectoryBtn;
    QToolButton* navSettingsBtn;
    QToolButton* navToolsBtn;
    QToolButton* navAboutBtn;
    QToolButton* currentNavButton;

    // UI组件 - 主内容区域
    QSplitter* mainSplitter;
    QStackedWidget* contentStack;

    // 目录选择页面
    QWidget* directoryPage;
    QLineEdit* dirPathLineEdit;
    QPushButton* dirPathButton;
    QLabel* directoryStatusLabel;
    QLabel* directoryStatusIcon;

    // 日志输出页面
    QWidget* logPage;
    QTextEdit* loggerTextEdit;
    QPushButton* clearLogBtn;
    QPushButton* exportLogBtn;

    // 设置页面
    QWidget* settingsPage;

    // 状态栏
    QLabel* statusLabel;
    QProgressBar* progressBar;

    // 核心组件
    ConfigManager* configManager;
    FfmpegManager* ffmpegManager;
    PatternManager* patternManager;
    FileScanner* fileScanner;

    // 菜单组件
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

    // 状态管理
    bool isDirectoryParsed;
    bool isMergingInProgress;
    QTimer* statusTimer;
};

#endif // MODERNMAINWINDOW_H