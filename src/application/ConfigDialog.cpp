#include "ConfigDialog.h"
#include "core/ConfigManager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDir>

ConfigDialog::ConfigDialog(ConfigManager *configManager, QWidget *parent)
    : QDialog(parent)
    , m_configManager(configManager)
{
    setWindowTitle(tr("设置"));
    setMinimumSize(500, 400);

    // 创建选项卡
    m_tabWidget = new QTabWidget(this);

    createBasicTab();
    createPathTab();
    createDanmakuTab();
    createCCSubtitleTab();

    // 创建按钮
    m_applyButton = new QPushButton(tr("应用"), this);
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_resetButton = new QPushButton(tr("重置"), this);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 加载配置到UI
    loadConfigToUI();

    // 连接信号槽
    connect(m_applyButton, &QPushButton::clicked, this, &ConfigDialog::onApplyClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ConfigDialog::onCancelClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &ConfigDialog::onResetClicked);
    connect(m_ffmpegPathBrowseButton, &QPushButton::clicked, this, &ConfigDialog::onFfmpegPathBrowseClicked);
    connect(m_patternPathBrowseButton, &QPushButton::clicked, this, &ConfigDialog::onPatternPathBrowseClicked);
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::createBasicTab()
{
    QWidget* basicTab = new QWidget(this);

    // 基本设置组
    QGroupBox* basicGroup = new QGroupBox(tr("基本设置"), basicTab);

    m_danmu2assCheckBox = new QCheckBox(tr("转换弹幕为ASS字幕"), basicGroup);
    m_coverSaveCheckBox = new QCheckBox(tr("保存封面"), basicGroup);
    m_ccDownCheckBox = new QCheckBox(tr("下载CC字幕"), basicGroup);
    m_oneDirCheckBox = new QCheckBox(tr("合并到同一目录"), basicGroup);
    m_overwriteCheckBox = new QCheckBox(tr("覆盖已存在的文件"), basicGroup);

    QVBoxLayout* basicLayout = new QVBoxLayout(basicGroup);
    basicLayout->addWidget(m_danmu2assCheckBox);
    basicLayout->addWidget(m_coverSaveCheckBox);
    basicLayout->addWidget(m_ccDownCheckBox);
    basicLayout->addWidget(m_oneDirCheckBox);
    basicLayout->addWidget(m_overwriteCheckBox);
    basicLayout->addStretch();

    QVBoxLayout* tabLayout = new QVBoxLayout(basicTab);
    tabLayout->addWidget(basicGroup);
    tabLayout->addStretch();

    basicTab->setLayout(tabLayout);
    m_tabWidget->addTab(basicTab, tr("基本"));
}

void ConfigDialog::createPathTab()
{
    QWidget* pathTab = new QWidget(this);

    // 自定义路径组
    QGroupBox* pathGroup = new QGroupBox(tr("自定义路径"), pathTab);

    m_customPermissionCheckBox = new QCheckBox(tr("启用自定义路径"), pathGroup);

    QLabel* ffmpegLabel = new QLabel(tr("FFmpeg路径:"), pathGroup);
    m_ffmpegPathLineEdit = new QLineEdit(pathGroup);
    m_ffmpegPathBrowseButton = new QPushButton(tr("浏览..."), pathGroup);

    QLabel* patternLabel = new QLabel(tr("模式文件路径:"), pathGroup);
    m_patternPathLineEdit = new QLineEdit(pathGroup);
    m_patternPathBrowseButton = new QPushButton(tr("浏览..."), pathGroup);

    // FFmpeg路径布局
    QHBoxLayout* ffmpegLayout = new QHBoxLayout();
    ffmpegLayout->addWidget(ffmpegLabel);
    ffmpegLayout->addWidget(m_ffmpegPathLineEdit);
    ffmpegLayout->addWidget(m_ffmpegPathBrowseButton);

    // 模式路径布局
    QHBoxLayout* patternLayout = new QHBoxLayout();
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(m_patternPathLineEdit);
    patternLayout->addWidget(m_patternPathBrowseButton);

    QVBoxLayout* pathLayout = new QVBoxLayout(pathGroup);
    pathLayout->addWidget(m_customPermissionCheckBox);
    pathLayout->addLayout(ffmpegLayout);
    pathLayout->addLayout(patternLayout);
    pathLayout->addStretch();

    QVBoxLayout* tabLayout = new QVBoxLayout(pathTab);
    tabLayout->addWidget(pathGroup);
    tabLayout->addStretch();

    pathTab->setLayout(tabLayout);
    m_tabWidget->addTab(pathTab, tr("路径"));
}

void ConfigDialog::createDanmakuTab()
{
    QWidget* danmakuTab = new QWidget(this);

    // 弹幕设置组
    QGroupBox* danmakuGroup = new QGroupBox(tr("弹幕设置"), danmakuTab);

    QFormLayout* formLayout = new QFormLayout(danmakuGroup);

    m_fontSizeSpinBox = new QSpinBox(danmakuGroup);
    m_fontSizeSpinBox->setRange(10, 100);
    m_fontSizeSpinBox->setSuffix(tr(" 像素"));
    formLayout->addRow(tr("字体大小:"), m_fontSizeSpinBox);

    m_textOpacitySpinBox = new QDoubleSpinBox(danmakuGroup);
    m_textOpacitySpinBox->setRange(0.0, 1.0);
    m_textOpacitySpinBox->setSingleStep(0.1);
    m_textOpacitySpinBox->setDecimals(2);
    formLayout->addRow(tr("文字透明度:"), m_textOpacitySpinBox);

    m_reverseBlankSpinBox = new QDoubleSpinBox(danmakuGroup);
    m_reverseBlankSpinBox->setRange(0.0, 1.0);
    m_reverseBlankSpinBox->setSingleStep(0.01);
    m_reverseBlankSpinBox->setDecimals(2);
    formLayout->addRow(tr("反向空白:"), m_reverseBlankSpinBox);

    m_durationMarqueeSpinBox = new QSpinBox(danmakuGroup);
    m_durationMarqueeSpinBox->setRange(1, 60);
    m_durationMarqueeSpinBox->setSuffix(tr(" 秒"));
    formLayout->addRow(tr("滚动弹幕持续时间:"), m_durationMarqueeSpinBox);

    m_durationStillSpinBox = new QSpinBox(danmakuGroup);
    m_durationStillSpinBox->setRange(1, 60);
    m_durationStillSpinBox->setSuffix(tr(" 秒"));
    formLayout->addRow(tr("静止弹幕持续时间:"), m_durationStillSpinBox);

    m_reduceCommentsCheckBox = new QCheckBox(tr("减少评论数量"), danmakuGroup);
    formLayout->addRow(tr("弹幕优化:"), m_reduceCommentsCheckBox);

    QVBoxLayout* tabLayout = new QVBoxLayout(danmakuTab);
    tabLayout->addWidget(danmakuGroup);
    tabLayout->addStretch();

    danmakuTab->setLayout(tabLayout);
    m_tabWidget->addTab(danmakuTab, tr("弹幕"));
}

void ConfigDialog::createCCSubtitleTab()
{
    QWidget* ccTab = new QWidget(this);

    QLabel* ccLabel = new QLabel(tr("CC字幕下载设置将在后续版本中实现"), ccTab);
    ccLabel->setAlignment(Qt::AlignCenter);
    ccLabel->setWordWrap(true);

    QVBoxLayout* tabLayout = new QVBoxLayout(ccTab);
    tabLayout->addWidget(ccLabel);
    tabLayout->addStretch();

    ccTab->setLayout(tabLayout);
    m_tabWidget->addTab(ccTab, tr("CC字幕"));
}

void ConfigDialog::loadConfigToUI()
{
    if (!m_configManager) return;

    // 基本设置
    m_danmu2assCheckBox->setChecked(m_configManager->danmu2ass());
    m_coverSaveCheckBox->setChecked(m_configManager->coverSave());
    m_ccDownCheckBox->setChecked(m_configManager->ccDown());
    m_oneDirCheckBox->setChecked(m_configManager->oneDir());
    m_overwriteCheckBox->setChecked(m_configManager->overwrite());

    // 路径设置
    m_customPermissionCheckBox->setChecked(m_configManager->customPermission());
    m_ffmpegPathLineEdit->setText(m_configManager->ffmpegPath());
    m_patternPathLineEdit->setText(m_configManager->patternFilePath());

    // 弹幕设置
    m_fontSizeSpinBox->setValue(m_configManager->fontSize());
    m_textOpacitySpinBox->setValue(m_configManager->textOpacity());
    m_reverseBlankSpinBox->setValue(m_configManager->reverseBlank());
    m_durationMarqueeSpinBox->setValue(m_configManager->durationMarquee());
    m_durationStillSpinBox->setValue(m_configManager->durationStill());
    m_reduceCommentsCheckBox->setChecked(m_configManager->isReduceComments());

    // 更新路径控件启用状态
    bool customEnabled = m_configManager->customPermission();
    m_ffmpegPathLineEdit->setEnabled(customEnabled);
    m_ffmpegPathBrowseButton->setEnabled(customEnabled);
    m_patternPathLineEdit->setEnabled(customEnabled);
    m_patternPathBrowseButton->setEnabled(customEnabled);

    connect(m_customPermissionCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_ffmpegPathLineEdit->setEnabled(checked);
        m_ffmpegPathBrowseButton->setEnabled(checked);
        m_patternPathLineEdit->setEnabled(checked);
        m_patternPathBrowseButton->setEnabled(checked);
    });
}

void ConfigDialog::saveConfigFromUI()
{
    if (!m_configManager) return;

    // 基本设置
    m_configManager->setDanmu2ass(m_danmu2assCheckBox->isChecked());
    m_configManager->setCoverSave(m_coverSaveCheckBox->isChecked());
    m_configManager->setCcDown(m_ccDownCheckBox->isChecked());
    m_configManager->setOneDir(m_oneDirCheckBox->isChecked());
    m_configManager->setOverwrite(m_overwriteCheckBox->isChecked());

    // 路径设置
    m_configManager->setCustomPermission(m_customPermissionCheckBox->isChecked());
    m_configManager->setFfmpegPath(m_ffmpegPathLineEdit->text());
    m_configManager->setPatternFilePath(m_patternPathLineEdit->text());

    // 弹幕设置
    m_configManager->setFontSize(m_fontSizeSpinBox->value());
    m_configManager->setTextOpacity(m_textOpacitySpinBox->value());
    m_configManager->setReverseBlank(m_reverseBlankSpinBox->value());
    m_configManager->setDurationMarquee(m_durationMarqueeSpinBox->value());
    m_configManager->setDurationStill(m_durationStillSpinBox->value());
    m_configManager->setIsReduceComments(m_reduceCommentsCheckBox->isChecked());
}

void ConfigDialog::onFfmpegPathBrowseClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择FFmpeg可执行文件"),
                                               QDir::homePath(),
                                               tr("可执行文件 (*.exe *);;所有文件 (*)"));
    if (!path.isEmpty()) {
        m_ffmpegPathLineEdit->setText(path);
    }
}

void ConfigDialog::onPatternPathBrowseClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("选择模式文件目录"),
                                                    QDir::homePath());
    if (!path.isEmpty()) {
        m_patternPathLineEdit->setText(path);
    }
}

void ConfigDialog::onApplyClicked()
{
    saveConfigFromUI();
    if (m_configManager->saveConfig()) {
        QMessageBox::information(this, tr("成功"), tr("设置已保存"));
        accept();
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法保存设置"));
    }
}

void ConfigDialog::onCancelClicked()
{
    reject();
}

void ConfigDialog::onResetClicked()
{
    if (QMessageBox::question(this, tr("确认"), tr("确定要重置所有设置吗？")) == QMessageBox::Yes) {
        m_configManager->resetToDefaults();
        loadConfigToUI();
    }
}