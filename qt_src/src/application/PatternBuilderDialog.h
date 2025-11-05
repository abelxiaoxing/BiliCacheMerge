#ifndef PATTERNBUILDERDIALOG_H
#define PATTERNBUILDERDIALOG_H

#include <QDialog>
#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProgressBar>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariantMap>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QMessageBox>

class ConfigManager;
class PatternManager;

/**
 * @brief 模式构建向导对话框
 * 参考Python版本的patternBuilding流程，提供交互式模式创建功能
 */
class PatternBuilderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatternBuilderDialog(ConfigManager *configManager, QWidget *parent = nullptr);
    ~PatternBuilderDialog();

private slots:
    void onBrowseClicked();
    void onScanClicked();
    void onNextClicked();
    void onBackClicked();
    void onFinishClicked();
    void onCancelClicked();
    void onFileItemDoubleClicked(QListWidgetItem *item);

private:
    // 初始化界面
    void createPages();
    void setupConnections();
    QWizardPage* createWizardPage(QWidget *widget);

    // 扫描和识别
    bool scanDirectory(const QString &path);
    void identifyFiles();
    QString getFileType(const QString &filePath);

    // 向导步骤
    void goToStep(int step);
    void updateStepIndicator();

    // UI填充方法
    void populateFileCombos();
    void populateMappingTree();
    void populateJsonTree();
    void parseJsonToTree(const QJsonObject &json, QTreeWidgetItem *parent);
    void populateFieldCombos();
    void extractFieldPaths(const QJsonObject &json, const QString &basePath, QStringList &paths);
    void updatePreview();

    // 模式生成
    void generatePattern();
    QString generateSearchTree();
    QString generateParseRules();
    bool savePatternFile();

    // UI组件
    QWizard *m_wizard;
    QWizardPage *m_stepPages[4];

    // 步骤1：目录选择
    QLineEdit *m_dirPathEdit;
    QPushButton *m_browseButton;
    QLabel *m_dirStatusLabel;
    QPushButton *m_scanButton;
    QProgressBar *m_scanProgress;
    QListWidget *m_fileListWidget;
    QLabel *m_recognizedFilesLabel;

    // 步骤2：文件映射
    QTreeWidget *m_mappingTree;
    QComboBox *m_videoFileCombo;
    QComboBox *m_audioFileCombo;
    QComboBox *m_entryFileCombo;
    QComboBox *m_danmuFileCombo;
    QComboBox *m_coverFileCombo;
    QCheckBox *m_hasGroupCheck;

    // 步骤3：字段映射
    QTreeWidget *m_jsonTree;
    QComboBox *m_aidFieldCombo;
    QComboBox *m_cidFieldCombo;
    QComboBox *m_titleFieldCombo;
    QComboBox *m_partTitleFieldCombo;
    QComboBox *m_coverUrlFieldCombo;

    // 步骤4：确认
    QTextEdit *m_previewText;
    QLineEdit *m_patternNameEdit;

    // 数据
    QString m_scanDirectory;
    QStringList m_allFiles;
    QMap<QString, QString> m_fileTypes; // 文件路径 -> 文件类型
    QString m_videoFile;
    QString m_audioFile;
    QString m_entryFile;
    QString m_danmuFile;
    QString m_coverFile;
    QString m_groupEntryFile;

    QJsonObject m_recognizedJson;
    QVariantMap m_generatedPattern;

    int m_currentStep;
    ConfigManager *m_configManager;

    // 常量
    static const int STEP_SELECT_DIR = 0;
    static const int STEP_MAP_FILES = 1;
    static const int STEP_MAP_FIELDS = 2;
    static const int STEP_CONFIRM = 3;
};

#endif // PATTERNBUILDERDIALOG_H
