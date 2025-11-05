#include "PatternBuilderDialog.h"
#include "core/ConfigManager.h"
#include "core/PatternManager.h"

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QScreen>

PatternBuilderDialog::PatternBuilderDialog(ConfigManager *configManager, QWidget *parent)
    : QDialog(parent)
    , m_configManager(configManager)
    , m_currentStep(0)
{
    setWindowTitle("搜索模式创建向导");
    setModal(true);
    resize(800, 600);

    m_wizard = new QWizard(this);
    createPages();
    setupConnections();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_wizard);

    updateStepIndicator();
}

PatternBuilderDialog::~PatternBuilderDialog()
{
}

void PatternBuilderDialog::createPages()
{
    // 步骤1：选择目录
    QWidget *page1 = new QWidget();
    QVBoxLayout *layout1 = new QVBoxLayout(page1);

    QLabel *titleLabel1 = new QLabel("步骤1: 选择包含缓存文件的目录");
    titleLabel1->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout1->addWidget(titleLabel1);

    QLabel *descLabel1 = new QLabel("请选择包含B站缓存文件的目录，程序将自动识别文件结构");
    layout1->addWidget(descLabel1);

    QHBoxLayout *dirLayout = new QHBoxLayout();
    m_dirPathEdit = new QLineEdit();
    m_browseButton = new QPushButton("浏览...");
    dirLayout->addWidget(m_dirPathEdit);
    dirLayout->addWidget(m_browseButton);
    layout1->addLayout(dirLayout);

    m_dirStatusLabel = new QLabel("未选择目录");
    m_dirStatusLabel->setStyleSheet("color: gray;");
    layout1->addWidget(m_dirStatusLabel);

    QPushButton *scanButton = new QPushButton("开始扫描");
    scanButton->setDefault(true);
    layout1->addWidget(scanButton);

    m_scanProgress = new QProgressBar();
    m_scanProgress->setVisible(false);
    layout1->addWidget(m_scanProgress);

    m_fileListWidget = new QListWidget();
    m_fileListWidget->setMaximumHeight(200);
    layout1->addWidget(m_fileListWidget);

    m_recognizedFilesLabel = new QLabel("未扫描");
    m_recognizedFilesLabel->setStyleSheet("font-weight: bold; color: blue;");
    layout1->addWidget(m_recognizedFilesLabel);

    layout1->addStretch();

    m_wizard->addPage(createWizardPage(page1));

    // 步骤2：文件映射
    QWidget *page2 = new QWidget();
    QVBoxLayout *layout2 = new QVBoxLayout(page2);

    QLabel *titleLabel2 = new QLabel("步骤2: 映射文件类型");
    titleLabel2->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout2->addWidget(titleLabel2);

    QLabel *descLabel2 = new QLabel("请指定每种类型的文件对应的文件");
    layout2->addWidget(descLabel2);

    QGridLayout *gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel("视频文件:"), 0, 0);
    m_videoFileCombo = new QComboBox();
    gridLayout->addWidget(m_videoFileCombo, 0, 1);

    gridLayout->addWidget(new QLabel("音频文件:"), 1, 0);
    m_audioFileCombo = new QComboBox();
    gridLayout->addWidget(m_audioFileCombo, 1, 1);

    gridLayout->addWidget(new QLabel("入口文件:"), 2, 0);
    m_entryFileCombo = new QComboBox();
    gridLayout->addWidget(m_entryFileCombo, 2, 1);

    gridLayout->addWidget(new QLabel("弹幕文件:"), 3, 0);
    m_danmuFileCombo = new QComboBox();
    gridLayout->addWidget(m_danmuFileCombo, 3, 1);

    gridLayout->addWidget(new QLabel("封面文件:"), 4, 0);
    m_coverFileCombo = new QComboBox();
    gridLayout->addWidget(m_coverFileCombo, 4, 1);

    layout2->addLayout(gridLayout);

    m_hasGroupCheck = new QCheckBox("此模式支持分组（多个视频的合集）");
    layout2->addWidget(m_hasGroupCheck);

    QLabel *mappingLabel = new QLabel("文件列表（双击选择）:");
    layout2->addWidget(mappingLabel);

    m_mappingTree = new QTreeWidget();
    m_mappingTree->setHeaderLabels(QStringList() << "文件" << "类型" << "路径");
    layout2->addWidget(m_mappingTree);

    layout2->addStretch();

    m_wizard->addPage(createWizardPage(page2));

    // 步骤3：字段映射
    QWidget *page3 = new QWidget();
    QVBoxLayout *layout3 = new QVBoxLayout(page3);

    QLabel *titleLabel3 = new QLabel("步骤3: 映射元数据字段");
    titleLabel3->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout3->addWidget(titleLabel3);

    QLabel *descLabel3 = new QLabel("请选择入口文件中对应字段的含义");
    layout3->addWidget(descLabel3);

    QJsonObject jsonObj = m_recognizedJson;
    QTreeWidgetItem *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, "entry.json");
    parseJsonToTree(jsonObj, rootItem);
    m_jsonTree = new QTreeWidget();
    m_jsonTree->addTopLevelItem(rootItem);
    m_jsonTree->expandAll();
    layout3->addWidget(m_jsonTree);

    QGridLayout *fieldLayout = new QGridLayout();

    fieldLayout->addWidget(new QLabel("视频ID (aid):"), 0, 0);
    m_aidFieldCombo = new QComboBox();
    fieldLayout->addWidget(m_aidFieldCombo, 0, 1);

    fieldLayout->addWidget(new QLabel("分集ID (cid):"), 1, 0);
    m_cidFieldCombo = new QComboBox();
    fieldLayout->addWidget(m_cidFieldCombo, 1, 1);

    fieldLayout->addWidget(new QLabel("标题:"), 2, 0);
    m_titleFieldCombo = new QComboBox();
    fieldLayout->addWidget(m_titleFieldCombo, 2, 1);

    fieldLayout->addWidget(new QLabel("分集标题:"), 3, 0);
    m_partTitleFieldCombo = new QComboBox();
    fieldLayout->addWidget(m_partTitleFieldCombo, 3, 1);

    fieldLayout->addWidget(new QLabel("封面URL:"), 4, 0);
    m_coverUrlFieldCombo = new QComboBox();
    fieldLayout->addWidget(m_coverUrlFieldCombo, 4, 1);

    layout3->addLayout(fieldLayout);
    layout3->addStretch();

    m_wizard->addPage(createWizardPage(page3));

    // 步骤4：确认
    QWidget *page4 = new QWidget();
    QVBoxLayout *layout4 = new QVBoxLayout(page4);

    QLabel *titleLabel4 = new QLabel("步骤4: 确认并保存");
    titleLabel4->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout4->addWidget(titleLabel4);

    QLabel *nameLabel = new QLabel("模式名称:");
    m_patternNameEdit = new QLineEdit();
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_patternNameEdit);
    layout4->addLayout(nameLayout);

    QLabel *previewLabel = new QLabel("模式预览:");
    layout4->addWidget(previewLabel);

    m_previewText = new QTextEdit();
    m_previewText->setReadOnly(true);
    layout4->addWidget(m_previewText);

    layout4->addStretch();

    m_wizard->addPage(createWizardPage(page4));

    // 连接按钮信号
    connect(m_browseButton, &QPushButton::clicked, this, &PatternBuilderDialog::onBrowseClicked);
    connect(m_scanButton, &QPushButton::clicked, this, &PatternBuilderDialog::onScanClicked);
    connect(m_fileListWidget, &QListWidget::itemDoubleClicked,
            this, &PatternBuilderDialog::onFileItemDoubleClicked);
}

QWizardPage *PatternBuilderDialog::createWizardPage(QWidget *widget)
{
    QWizardPage *page = new QWizardPage();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(widget);
    return page;
}

void PatternBuilderDialog::setupConnections()
{
    connect(m_wizard, &QWizard::currentIdChanged, this, [this](int id) {
        m_currentStep = id;
        updateStepIndicator();

        if (id == STEP_MAP_FILES) {
            populateFileCombos();
            populateMappingTree();
        } else if (id == STEP_MAP_FIELDS) {
            populateJsonTree();
            populateFieldCombos();
        } else if (id == STEP_CONFIRM) {
            generatePattern();
            updatePreview();
        }
    });

    connect(m_wizard->button(QWizard::NextButton), &QPushButton::clicked, this, &PatternBuilderDialog::onNextClicked);
    connect(m_wizard->button(QWizard::BackButton), &QPushButton::clicked, this, &PatternBuilderDialog::onBackClicked);
    connect(m_wizard->button(QWizard::FinishButton), &QPushButton::clicked, this, &PatternBuilderDialog::onFinishClicked);
    connect(m_wizard->button(QWizard::CancelButton), &QPushButton::clicked, this, &PatternBuilderDialog::onCancelClicked);
}

void PatternBuilderDialog::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择目录", m_dirPathEdit->text());
    if (!dir.isEmpty()) {
        m_dirPathEdit->setText(dir);
        m_dirStatusLabel->setText(QString("已选择: %1").arg(dir));
        m_dirStatusLabel->setStyleSheet("color: green;");
    }
}

void PatternBuilderDialog::onScanClicked()
{
    QString dir = m_dirPathEdit->text();
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择目录");
        return;
    }

    m_scanDirectory = dir;
    scanDirectory(dir);
}

bool PatternBuilderDialog::scanDirectory(const QString &path)
{
    m_scanProgress->setVisible(true);
    m_scanProgress->setRange(0, 0); // 不确定进度条
    m_scanProgress->setValue(0);

    m_fileListWidget->clear();
    m_allFiles.clear();
    m_fileTypes.clear();

    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    int count = 0;
    for (const QFileInfo &entry : entries) {
        if (entry.isFile()) {
            QString filePath = entry.absoluteFilePath();
            QString fileType = getFileType(filePath);

            m_allFiles.append(filePath);
            m_fileTypes[filePath] = fileType;

            QListWidgetItem *item = new QListWidgetItem(entry.fileName());
            item->setData(Qt::UserRole, filePath);
            item->setText(QString("%1 (%2)").arg(entry.fileName()).arg(fileType));
            m_fileListWidget->addItem(item);

            count++;
        }

        QApplication::processEvents();
    }

    m_scanProgress->setVisible(false);
    m_recognizedFilesLabel->setText(QString("识别到 %1 个文件").arg(count));

    identifyFiles();

    return true;
}

QString PatternBuilderDialog::getFileType(const QString &filePath)
{
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();

    if (suffix == "json") {
        return "JSON文件";
    } else if (suffix == "xml") {
        return "弹幕文件";
    } else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png") {
        return "图片文件";
    } else if (suffix == "mp4" || suffix == "m4s" || suffix == "flv") {
        return "视频文件";
    } else if (suffix == "mp3" || suffix == "m4a" || suffix == "aac") {
        return "音频文件";
    } else {
        return "未知类型";
    }
}

void PatternBuilderDialog::identifyFiles()
{
    // 识别入口文件
    for (const QString &filePath : m_allFiles) {
        if (m_fileTypes[filePath] == "JSON文件") {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
                if (error.error == QJsonParseError::NoError && doc.isObject()) {
                    m_recognizedJson = doc.object();
                    file.close();
                    break;
                }
                file.close();
            }
        }
    }
}

void PatternBuilderDialog::populateFileCombos()
{
    QStringList videoFiles, audioFiles, entryFiles, danmuFiles, coverFiles;

    for (const QString &filePath : m_allFiles) {
        QString fileName = QFileInfo(filePath).fileName();
        QString fileType = m_fileTypes[filePath];

        if (fileType == "视频文件") {
            videoFiles.append(filePath);
        } else if (fileType == "音频文件") {
            audioFiles.append(filePath);
        } else if (fileType == "JSON文件") {
            entryFiles.append(filePath);
        } else if (fileType == "弹幕文件") {
            danmuFiles.append(filePath);
        } else if (fileType == "图片文件") {
            coverFiles.append(filePath);
        }
    }

    m_videoFileCombo->clear();
    m_videoFileCombo->addItems(videoFiles);

    m_audioFileCombo->clear();
    m_audioFileCombo->addItems(audioFiles);

    m_entryFileCombo->clear();
    m_entryFileCombo->addItems(entryFiles);

    m_danmuFileCombo->clear();
    m_danmuFileCombo->addItem("无", "");
    m_danmuFileCombo->addItems(danmuFiles);

    m_coverFileCombo->clear();
    m_coverFileCombo->addItem("无", "");
    m_coverFileCombo->addItems(coverFiles);
}

void PatternBuilderDialog::populateMappingTree()
{
    m_mappingTree->clear();

    for (const QString &filePath : m_allFiles) {
        QString fileName = QFileInfo(filePath).fileName();
        QString fileType = m_fileTypes[filePath];

        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, fileName);
        item->setText(1, fileType);
        item->setText(2, filePath);
        item->setData(0, Qt::UserRole, filePath);

        m_mappingTree->addTopLevelItem(item);
    }
}

void PatternBuilderDialog::onFileItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    // 双击文件时的处理逻辑
    m_wizard->next();
}

void PatternBuilderDialog::populateJsonTree()
{
    m_jsonTree->clear();

    if (m_recognizedJson.isEmpty()) {
        return;
    }

    QTreeWidgetItem *rootItem = new QTreeWidgetItem();
    rootItem->setText(0, "entry.json");
    parseJsonToTree(m_recognizedJson, rootItem);
    m_jsonTree->addTopLevelItem(rootItem);
    m_jsonTree->expandAll();
}

void PatternBuilderDialog::parseJsonToTree(const QJsonObject &json, QTreeWidgetItem *parent)
{
    for (auto it = json.begin(); it != json.end(); ++it) {
        QTreeWidgetItem *childItem = new QTreeWidgetItem(parent);
        childItem->setText(0, it.key());

        if (it->isObject()) {
            parseJsonToTree(it->toObject(), childItem);
        } else if (it->isArray()) {
            QJsonArray arr = it->toArray();
            childItem->setText(1, QString("数组[%1项]").arg(arr.size()));
        } else if (it->isString()) {
            childItem->setText(1, it->toString());
        } else if (it->isDouble()) {
            childItem->setText(1, QString::number(it->toDouble()));
        } else if (it->isBool()) {
            childItem->setText(1, it->toBool() ? "true" : "false");
        }

        parent->addChild(childItem);
    }
}

void PatternBuilderDialog::populateFieldCombos()
{
    QStringList fieldPaths;
    extractFieldPaths(m_recognizedJson, "", fieldPaths);

    m_aidFieldCombo->clear();
    m_aidFieldCombo->addItems(fieldPaths);

    m_cidFieldCombo->clear();
    m_cidFieldCombo->addItems(fieldPaths);

    m_titleFieldCombo->clear();
    m_titleFieldCombo->addItems(fieldPaths);

    m_partTitleFieldCombo->clear();
    m_partTitleFieldCombo->addItems(fieldPaths);

    m_coverUrlFieldCombo->clear();
    m_coverUrlFieldCombo->addItems(fieldPaths);
}

void PatternBuilderDialog::extractFieldPaths(const QJsonObject &json, const QString &basePath, QStringList &paths)
{
    for (auto it = json.begin(); it != json.end(); ++it) {
        QString currentPath = basePath.isEmpty() ? it.key() : basePath + "-" + it.key();

        if (it->isObject()) {
            extractFieldPaths(it->toObject(), currentPath, paths);
        } else {
            paths.append(currentPath);
        }
    }
}

void PatternBuilderDialog::generatePattern()
{
    QString patternName = m_patternNameEdit->text().trimmed();
    if (patternName.isEmpty()) {
        patternName = "新模式";
        m_patternNameEdit->setText(patternName);
    }

    QJsonObject pattern;
    pattern["name"] = patternName;

    // 生成搜索规则
    QJsonObject search;
    search["has_group"] = m_hasGroupCheck->isChecked() ? 1 : 0;
    search["name"] = generateSearchTree();
    pattern["search"] = search;

    // 生成解析规则
    QJsonObject parse;
    parse["aid"] = m_aidFieldCombo->currentText();
    parse["cid"] = m_cidFieldCombo->currentText();
    parse["title"] = m_titleFieldCombo->currentText();
    parse["part_title"] = m_partTitleFieldCombo->currentText();
    parse["cover_url"] = m_coverUrlFieldCombo->currentText();
    pattern["parse"] = parse;

    m_generatedPattern = QJsonDocument(pattern).toVariant().toMap();
}

QString PatternBuilderDialog::generateSearchTree()
{
    // 生成tree结构的简化实现
    return "entry.json";
}

void PatternBuilderDialog::updatePreview()
{
    QJsonDocument doc = QJsonDocument::fromVariant(m_generatedPattern);
    m_previewText->setPlainText(doc.toJson(QJsonDocument::Indented));
}

bool PatternBuilderDialog::savePatternFile()
{
    if (m_generatedPattern.isEmpty()) {
        return false;
    }

    QString patternName = m_patternNameEdit->text().trimmed();
    if (patternName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入模式名称");
        return false;
    }

    QString patternDir = m_configManager->defaultPatternPath();
    QDir dir(patternDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = dir.filePath(patternName + ".pat");

    QJsonDocument doc = QJsonDocument::fromVariant(m_generatedPattern);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        return true;
    }

    return false;
}

void PatternBuilderDialog::onNextClicked()
{
    if (m_currentStep == STEP_MAP_FILES) {
        // 验证文件映射
        if (m_videoFileCombo->currentText().isEmpty() ||
            m_audioFileCombo->currentText().isEmpty() ||
            m_entryFileCombo->currentText().isEmpty()) {
            QMessageBox::warning(this, "警告", "请至少指定视频、音频和入口文件");
            m_wizard->back();
        }
    } else if (m_currentStep == STEP_MAP_FIELDS) {
        // 验证字段映射
        if (m_aidFieldCombo->currentText().isEmpty() ||
            m_cidFieldCombo->currentText().isEmpty()) {
            QMessageBox::warning(this, "警告", "请至少指定aid和cid字段");
            m_wizard->back();
        }
    }
}

void PatternBuilderDialog::onBackClicked()
{
}

void PatternBuilderDialog::onFinishClicked()
{
    if (savePatternFile()) {
        QMessageBox::information(this, "成功", "模式文件已保存！");
        accept();
    } else {
        QMessageBox::critical(this, "错误", "保存失败！");
    }
}

void PatternBuilderDialog::onCancelClicked()
{
    reject();
}

void PatternBuilderDialog::updateStepIndicator()
{
    QString title = QString("步骤 %1/4").arg(m_currentStep + 1);
    setWindowTitle(QString("搜索模式创建向导 - %1").arg(title));
}


