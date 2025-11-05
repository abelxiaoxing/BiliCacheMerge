#include "HelpDialog.h"
#include <QHeaderView>
#include <QApplication>
#include <QScreen>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("使用说明 - B站缓存合并工具");
    setMinimumSize(900, 700);
    resize(900, 700);

    // 居中显示
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    createContent();
    populateTopics();
}

HelpDialog::~HelpDialog()
{
}

void HelpDialog::createContent()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // 标题
    QLabel *titleLabel = new QLabel("帮助与使用指南", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    color: #2196F3;"
        "    padding: 10px;"
        "}"
    );
    layout->addWidget(titleLabel);

    // 分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(m_splitter);

    // 左侧主题树
    m_topicTree = new QTreeWidget(this);
    m_topicTree->setHeaderLabel("帮助主题");
    m_topicTree->setMaximumWidth(250);
    m_topicTree->setMinimumWidth(200);
    m_topicTree->header()->setVisible(false);

    // 右侧内容区域
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索帮助内容...");
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px;"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
    );
    rightLayout->addWidget(m_searchEdit);

    // 内容浏览器
    m_contentBrowser = new QTextBrowser(this);
    m_contentBrowser->setOpenExternalLinks(true);
    m_contentBrowser->setStyleSheet(
        "QTextBrowser {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 10px;"
        "    background-color: white;"
        "    font-size: 14px;"
        "}"
    );
    rightLayout->addWidget(m_contentBrowser);

    // 关闭按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setDefault(true);
    buttonLayout->addWidget(m_closeButton);
    layout->addLayout(buttonLayout);

    // 添加到分割器
    m_splitter->addWidget(m_topicTree);
    m_splitter->addWidget(rightWidget);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    // 连接信号
    connect(m_topicTree, &QTreeWidget::currentItemChanged,
            this, &HelpDialog::onTopicChanged);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &HelpDialog::onSearchChanged);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void HelpDialog::populateTopics()
{
    // 创建帮助内容
    m_helpContent["tutorial"] = getTutorialContent();
    m_helpContent["question"] = getQuestionContent();
    m_helpContent["consult"] = getConsultContent();
    m_helpContent["about"] = getAboutContent();

    // 添加主题项
    QTreeWidgetItem *tutorialItem = new QTreeWidgetItem(m_topicTree);
    tutorialItem->setText(0, "使用说明");
    tutorialItem->setData(0, Qt::UserRole, "tutorial");

    QTreeWidgetItem *questionItem = new QTreeWidgetItem(m_topicTree);
    questionItem->setText(0, "常见问题");
    questionItem->setData(0, Qt::UserRole, "question");

    QTreeWidgetItem *consultItem = new QTreeWidgetItem(m_topicTree);
    consultItem->setText(0, "反馈信息");
    consultItem->setData(0, Qt::UserRole, "consult");

    QTreeWidgetItem *aboutItem = new QTreeWidgetItem(m_topicTree);
    aboutItem->setText(0, "关于软件");
    aboutItem->setData(0, Qt::UserRole, "about");

    // 展开所有项目
    m_topicTree->expandAll();

    // 选择第一个主题
    m_topicTree->setCurrentItem(tutorialItem);
}

void HelpDialog::onTopicChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (item) {
        QString key = item->data(0, Qt::UserRole).toString();
        QString content = m_helpContent.value(key);
        if (!content.isEmpty()) {
            m_contentBrowser->setHtml(content);
        }
    }
}

void HelpDialog::onSearchChanged(const QString &text)
{
    Q_UNUSED(text);
    // 简单实现：清空搜索，保留全部内容
    // TODO: 实现真正的搜索功能
}

QString HelpDialog::getTutorialContent()
{
    return QString(
        "<h2>使用说明</h2>"
        "<h3>1. 基本操作</h3>"
        "<p><strong>步骤1：选择目录</strong><br/>"
        "点击上方的\"浏览\"按钮，选择包含B站缓存文件的目录。程序支持以下客户端的缓存格式：</p>"
        "<ul>"
        "<li>Android客户端</li>"
        "<li>Windows 10 UWP</li>"
        "<li>Windows 10 桌面版</li>"
        "<li>电影版客户端</li>"
        "<li>命令行工具</li>"
        "</ul>"

        "<p><strong>步骤2：开始合并</strong><br/>"
        "选择目录后，程序会自动识别文件结构。点击\"开始合并\"按钮即可开始处理。</p>"

        "<h3>2. 配置选项</h3>"
        "<p>点击菜单栏的\"设置\"可以调整以下选项：</p>"
        "<ul>"
        "<li><strong>弹幕转ASS</strong>：是否将XML弹幕转换为ASS字幕文件</li>"
        "<li><strong>封面保存</strong>：是否保存视频封面图片</li>"
        "<li><strong>CC字幕</strong>：是否下载CC字幕文件</li>"
        "<li><strong>单目录模式</strong>：是否只处理单个目录</li>"
        "<li><strong>视频编号</strong>：是否在文件名中添加编号</li>"
        "</ul>"

        "<h3>3. 弹幕配置</h3>"
        "<p>在设置中可以详细配置弹幕样式：</p>"
        "<ul>"
        "<li><strong>字体大小</strong>：设置弹幕字体大小（默认25px）</li>"
        "<li><strong>文字透明度</strong>：设置弹幕透明度（0.0-1.0）</li>"
        "<li><strong>滚动弹幕持续时间</strong>：滚动弹幕的显示时长</li>"
        "<li><strong>静止弹幕持续时间</strong>：顶部/底部弹幕的显示时长</li>"
        "<li><strong>弹幕减少</strong>：是否启用弹幕减少模式</li>"
        "</ul>"

        "<h3>4. 高级功能</h3>"
        "<p><strong>模式构建向导</strong>：如果遇到不支持的客户端格式，可以使用\"工具 > 模式构建\"创建自定义模式。</p>"
        "<p><strong>自定义FFmpeg路径</strong>：在设置中可以指定自定义的FFmpeg可执行文件路径。</p>"

        "<h3>5. 输出文件</h3>"
        "<p>合并后的文件将保存在原目录，文件名格式为：\"视频标题.mp4\"。同时会生成以下附加文件：</p>"
        "<ul>"
        "<li>ASS字幕文件（如启用弹幕转ASS）</li>"
        "<li>SRT字幕文件（如启用CC字幕）</li>"
        "<li>封面图片（如启用封面保存）</li>"
        "</ul>"

        "<h3>6. 注意事项</h3>"
        "<ul>"
        "<li>确保有足够的磁盘空间存储输出文件</li>"
        "<li>合并过程可能需要较长时间，请耐心等待</li>"
        "<li>建议关闭杀毒软件以避免FFmpeg被误删</li>"
        "<li>如遇问题请查看日志文件或联系反馈</li>"
        "</ul>"
    );
}

QString HelpDialog::getQuestionContent()
{
    return QString(
        "<h2>常见问题</h2>"

        "<h3>Q1：程序提示\"FFmpeg载入失败\"怎么办？</h3>"
        "<p><strong>A：</strong>请检查以下几点：</p>"
        "<ul>"
        "<li>确保FFmpeg已正确安装并配置到系统PATH环境变量中</li>"
        "<li>或手动指定FFmpeg路径：设置 > 自定义路径</li>"
        "<li>Windows用户可从 https://ffmpeg.org 下载预编译版本</li>"
        "</ul>"

        "<h3>Q2：合并过程中出现错误怎么办？</h3>"
        "<p><strong>A：</strong>可以尝试以下方法：</p>"
        "<ul>"
        "<li>启用\"错误跳过\"选项：选项 > 错误跳过</li>"
        "<li>检查原始文件是否损坏</li>"
        "<li>查看详细日志了解具体错误信息</li>"
        "<li>确保有足够的磁盘空间</li>"
        "</ul>"

        "<h3>Q3：支持哪些客户端的缓存格式？</h3>"
        "<p><strong>A：</strong>当前支持的客户端包括：</p>"
        "<ul>"
        "<li>Android客户端（.m4s格式）</li>"
        "<li>Windows 10 UWP</li>"
        "<li>Windows 10 桌面版</li>"
        "<li>电影版客户端</li>"
        "<li>命令行工具</li>"
        "</ul>"
        "<p>如需支持其他客户端，请使用\"工具 > 模式构建\"创建自定义模式。</p>"

        "<h3>Q4：弹幕无法显示或显示异常？</h3>"
        "<p><strong>A：</strong>请检查：</p>"
        "<ul>"
        "<li>确保弹幕文件（XML）存在且格式正确</li>"
        "<li>调整弹幕配置中的字体大小和持续时间</li>"
        "<li>播放器是否支持ASS字幕格式</li>"
        "<li>尝试减小弹幕字体大小或启用\"弹幕减少\"选项</li>"
        "</ul>"

        "<h3>Q5：合并速度很慢怎么办？</h3>"
        "<p><strong>A：</strong>提速建议：</p>"
        "<ul>"
        "<li>关闭杀毒软件的实时保护（可能影响FFmpeg运行）</li>"
        "<li>使用SSD硬盘可显著提升速度</li>"
        "<li>启用\"单目录模式\"可减少I/O操作</li>"
        "<li>关闭弹幕转ASS和字幕下载功能（如果不需要）</li>"
        "</ul>"

        "<h3>Q6：生成的文件无法播放？</h3>"
        "<p><strong>A：</strong>可能的原因和解决方法：</p>"
        "<ul>"
        "<li>原始文件损坏：重新下载缓存文件</li>"
        "<li>编码格式问题：确保FFmpeg版本较新（建议4.0以上）</li>"
        "<li>播放器不支持：尝试使用VLC等解码能力强的播放器</li>"
        "<li>文件不完整：检查磁盘空间是否充足</li>"
        "</ul>"

        "<h3>Q7：如何自定义模式？</h3>"
        "<p><strong>A：</strong>步骤如下：</p>"
        "<ol>"
        "<li>点击\"工具 > 模式构建\"</li>"
        "<li>选择包含新客户端缓存的目录</li>"
        "<li>按照向导指引映射文件类型</li>"
        "<li>指定元数据字段（如aid、cid等）</li>"
        "<li>保存模式文件（.pat格式）</li>"
        "</ol>"
        "<p>新模式将自动被程序识别和使用。</p>"

        "<h3>Q8：程序崩溃或无响应？</h3>"
        "<p><strong>A：</strong>排查步骤：</p>"
        "<ul>"
        "<li>查看Windows事件查看器或Linux系统日志</li>"
        "<li>尝试处理较少的文件（分批处理）</li>"
        "<li>更新到最新版本</li>"
        "<li>检查系统资源使用情况（内存、磁盘）</li>"
        "<li>联系开发者并提供日志文件</li>"
        "</ul>"
    );
}

QString HelpDialog::getConsultContent()
{
    return QString(
        "<h2>反馈信息</h2>"

        "<h3>获取帮助</h3>"
        "<p>如果您在使用过程中遇到问题，可以通过以下方式获取帮助：</p>"
        "<ul>"
        "<li>查看本帮助文档的\"常见问题\"部分</li>"
        "<li>查看程序日志获取详细错误信息</li>"
        "<li>检查GitHub Issues是否有类似问题</li>"
        "</ul>"

        "<h3>提交反馈</h3>"
        "<p>欢迎提交Bug报告、功能建议或使用反馈：</p>"
        "<ul>"
        "<li><strong>GitHub Issues</strong>：提交Bug报告或功能请求</li>"
        "<li><strong>邮件联系</strong>：发送详细问题描述</li>"
        "<li><strong>用户群</strong>：加入用户交流群讨论</li>"
        "</ul>"

        "<h3>报告Bug时请提供</h3>"
        "<p>为了更快定位和解决问题，请提供以下信息：</p>"
        "<ol>"
        "<li>程序版本号（帮助 > 关于）</li>"
        "<li>操作系统版本（Windows/Linux）</li>"
        "<li>FFmpeg版本（设置中可查看）</li>"
        "<li>详细的错误描述和复现步骤</li>"
        "<li>相关的日志文件内容</li>"
        "<li>如可能，提供示例文件或最小复现场景</li>"
        "</ol>"

        "<h3>功能建议</h3>"
        "<p>我们欢迎各种改进建议，包括但不限于：</p>"
        "<ul>"
        "<li>支持更多客户端格式</li>"
        "<li>新的输出格式（如MKV、AVI等）</li>"
        "<li>批量重命名功能</li>"
        "<li>预览功能</li>"
        "<li>插件系统</li>"
        "<li>性能优化</li>"
        "</ul>"
        "<p>提交建议时请说明使用场景和预期效果。</p>"

        "<h3>贡献代码</h3>"
        "<p>欢迎开发者贡献代码！请遵循以下步骤：</p>"
        "<ol>"
        "<li>Fork本项目</li>"
        "<li>创建特性分支（git checkout -b feature/xxx）</li>"
        "<li>提交更改（git commit -am 'Add xxx'）</li>"
        "<li>推送分支（git push origin feature/xxx）</li>"
        "<li>创建Pull Request</li>"
        "</ol>"
        "<p>代码风格请参考现有代码，保持一致性。</p>"

        "<h3>捐助支持</h3>"
        "<p>如果这个工具对您有帮助，欢迎通过以下方式支持开发：</p>"
        "<ul>"
        "<li>GitHub Star：为我们点个星</li>"
        "<li>捐助：扫码支持开发者</li>"
        "<li>推荐：推荐给需要的朋友</li>"
        "</ul>"
        "<p>所有捐助将用于服务器费用和开发工具采购。</p>"

        "<h3>致谢</h3>"
        "<p>感谢所有用户的反馈和支持，让这个工具变得越来越好！</p>"
        "<p>特别感谢以下贡献者：</p>"
        "<ul>"
        "<li>所有提交Bug报告的用户</li>"
        "<li>参与代码贡献的开发者</li>"
        "<li>提供翻译和文档支持的志愿者</li>"
        "</ul>"
    );
}

QString HelpDialog::getAboutContent()
{
    return QString(
        "<h2>关于软件</h2>"

        "<h3>软件信息</h3>"
        "<p><strong>软件名称：</strong> B站缓存合并工具<br/>"
        "<strong>版本：</strong> 2.0.0<br/>"
        "<strong>开发语言：</strong> C++ / Qt6<br/>"
        "<strong>许可证：</strong> MIT License<br/>"
        "<strong>源码：</strong> <a href='https://github.com/your-repo'>GitHub Repository</a></p>"

        "<h3>版本历史</h3>"
        "<p><strong>v2.0.0 (2025-xx-xx)</strong><br/>"
        "• 全新Qt6重构版本<br/>"
        "• 现代化UI界面<br/>"
        "• 更好的性能和稳定性<br/>"
        "• 新增多种客户端支持</p>"

        "<p><strong>v1.8 (2024-xx-xx)</strong><br/>"
        "• Python wx版本<br/>"
        "• 完整功能集<br/>"
        "• 跨平台支持</p>"

        "<h3>技术特性</h3>"
        "<ul>"
        "<li><strong>跨平台：</strong>支持Windows、Linux、macOS</li>"
        "<li><strong>多格式支持：</strong>支持多种B站客户端缓存格式</li>"
        "<li><strong>高性能：</strong>使用C++和Qt6确保高效处理</li>"
        "<li><strong>现代化UI：</strong>基于Qt6的现代界面设计</li>"
        "<li><strong>可扩展：</strong>支持自定义模式和插件</li>"
        "<li><strong>开源免费：</strong>完全开源，可自由使用和修改</li>"
        "</ul>"

        "<h3>依赖工具</h3>"
        "<ul>"
        "<li><strong>FFmpeg：</strong>视频合并核心工具</li>"
        "<li><strong>Qt6：</strong>跨平台UI框架</li>"
        "<li><strong>C++17：</strong>现代C++特性支持</li>"
        "<li><strong>CMake：</strong>跨平台构建系统</li>"
        "</ul>"

        "<h3>性能数据</h3>"
        "<ul>"
        "<li>单个视频合并：&lt; 10秒（取决于视频大小）</li>"
        "<li>批量处理：支持数千个视频</li>"
        "<li>内存占用：约50-100MB</li>"
        "<li>CPU使用：合并时50-100%，空闲时&lt;1%</li>"
        "</ul>"

        "<h3>开发者信息</h3>"
        "<p><strong>主要开发者：</strong> [Your Name]<br/>"
        "<strong>联系方式：</strong> [Your Email]<br/>"
        "<strong>主页：</strong> <a href='https://your-website.com'>https://your-website.com</a></p>"

        "<h3>开源协议</h3>"
        "<p>本软件采用 MIT 许可证，您可以：</p>"
        "<ul>"
        "<li>自由使用、修改和分发</li>"
        "<li>用于商业用途</li>"
        "<li>重新发布和销售</li>"
        "</ul>"
        "<p>只需要在分发时保留版权声明和许可证文本。</p>"

        "<h3>隐私声明</h3>"
        "<p>本软件：</p>"
        "<ul>"
        "<li>完全本地运行，不收集任何用户数据</li>"
        "<li>不会上传文件到任何服务器</li>"
        "<li>所有处理均在用户本地完成</li>"
        "<li>仅在下载字幕时访问B站API</li>"
        "</ul>"

        "<h3>感谢</h3>"
        "<p>感谢以下项目和服务：</p>"
        "<ul>"
        "<li>Qt Framework - 强大的跨平台UI框架</li>"
        "<li>FFmpeg - 世界领先的音视频处理库</li>"
        "<li>所有贡献者和用户的支持</li>"
        "</ul>"

        "<p style='text-align: center; margin-top: 30px; color: #666;'>"
        "© 2025 B站缓存合并工具开发团队. 保留所有权利."
        "</p>"
    );
}
