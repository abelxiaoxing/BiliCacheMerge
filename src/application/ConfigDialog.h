#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGroupBox>

class ConfigManager;

/**
 * @brief 设置对话框类
 * 对应Python版本的configFrame类
 *
 * 包含多个选项卡:
 * - 基本设置
 * - 路径配置
 * - 弹幕设置
 * - CC字幕设置
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(ConfigManager *configManager, QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void onFfmpegPathBrowseClicked();
    void onPatternPathBrowseClicked();
    void onApplyClicked();
    void onCancelClicked();
    void onResetClicked();

private:
    void createBasicTab();
    void createPathTab();
    void createDanmakuTab();
    void createCCSubtitleTab();
    void loadConfigToUI();
    void saveConfigFromUI();

    ConfigManager* m_configManager;

    // UI组件 - 基本设置
    QCheckBox* m_danmu2assCheckBox;
    QCheckBox* m_coverSaveCheckBox;
    QCheckBox* m_ccDownCheckBox;
    QCheckBox* m_oneDirCheckBox;
    QCheckBox* m_overwriteCheckBox;

    // UI组件 - 路径配置
    QLineEdit* m_ffmpegPathLineEdit;
    QPushButton* m_ffmpegPathBrowseButton;
    QLineEdit* m_patternPathLineEdit;
    QPushButton* m_patternPathBrowseButton;
    QCheckBox* m_customPermissionCheckBox;

    // UI组件 - 弹幕设置
    QSpinBox* m_fontSizeSpinBox;
    QDoubleSpinBox* m_textOpacitySpinBox;
    QDoubleSpinBox* m_reverseBlankSpinBox;
    QSpinBox* m_durationMarqueeSpinBox;
    QSpinBox* m_durationStillSpinBox;
    QCheckBox* m_reduceCommentsCheckBox;

    // 主要组件
    QTabWidget* m_tabWidget;
    QPushButton* m_applyButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
};

#endif // CONFIGDIALOG_H