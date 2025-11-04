#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include <QMessageBox>
#include <QScrollBar>
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
    QLineEdit* dirPathLineEdit;
    QPushButton* dirPathButton;
    QTextEdit* loggerTextEdit;
    QProgressBar* progressBar;

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