#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <memory>

QT_BEGIN_NAMESPACE
class QMenuBar;
class QMenu;
class QAction;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QStatusBar;
class QGroupBox;
class QSplitter;
QT_END_NAMESPACE

class DropLineEdit;
class MergeWorker;

/**
 * @brief 主窗口类
 *
 * 应用程序的主界面，包含文件选择、模式设置、日志显示等功能
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // 菜单动作槽函数
    void onStartMerge();
    void onPauseMerge();
    void onResumeMerge();
    void onSettings();
    void onPatternBuilder();
    void onJsonCheck();
    void onShowTutorial();
    void onShowFAQ();
    void onShowFeedback();
    void onShowAbout();
    void onShowLog();
    void onQuit();

    // 核心功能槽函数
    void onSelectDirectory();
    void onMergeModeChanged();
    void onMergeStarted();
    void onMergeFinished();
    void onMergeProgressUpdated(int progress, const QString &message);
    void onMergeErrorOccurred(const QString &error);

    // UI更新槽函数
    void updateUI();
    void updateProgress();

private:
    void setupUI();
    void setupMenuBar();
    void setupCentralWidget();
    void setupStatusBar();
    void connectSignals();
    void loadSettings();
    void saveSettings();

    // UI组件创建
    QWidget* createHeaderWidget();
    QWidget* createControlWidget();
    QWidget* createLogWidget();
    QGroupBox* createFileSelectionGroup();
    QGroupBox* createModeSelectionGroup();
    QGroupBox* createActionGroup();

    // 成员变量
    // 菜单和动作
    QMenuBar *m_menuBar;
    QMenu *m_menuOptions;
    QMenu *m_menuTools;
    QMenu *m_menuHelp;
    QMenu *m_menuAbout;

    QAction *m_actionStart;
    QAction *m_actionPause;
    QAction *m_actionResume;
    QAction *m_actionSettings;
    QAction *m_actionQuit;
    QAction *m_actionPatternBuilder;
    QAction *m_actionJsonCheck;
    QAction *m_actionTutorial;
    QAction *m_actionFAQ;
    QAction *m_actionFeedback;
    QAction *m_actionAbout;
    QAction *m_actionLog;

    // 主要UI组件
    DropLineEdit *m_pathEdit;
    QPushButton *m_browseButton;
    QComboBox *m_modeComboBox;
    QPushButton *m_mergeButton;
    QTextEdit *m_logTextEdit;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_progressLabel;

    // 布局组件
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 工作线程
    std::unique_ptr<MergeWorker> m_mergeWorker;
    QTimer *m_updateTimer;

    // 状态变量
    bool m_isMerging;
    bool m_isPaused;
    QString m_currentDirectory;
    QString m_lastPattern;
};

#endif // MAINWINDOW_H