#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>

/**
 * @brief 图标管理器 - 提供统一的高质量图标资源
 * 支持多种尺寸和风格的主题图标
 */
class IconManager
{
public:
    enum IconType {
        // 主要操作图标 (64x64px)
        ICON_PLAY = 0,           // 开始合并/播放
        ICON_FOLDER,             // 文件夹/目录选择
        ICON_SETTINGS,            // 设置

        // 导航图标 (32x32px)
        ICON_DIRECTORY,          // 导航目录
        ICON_GEAR,               // 导航设置
        ICON_TOOLS,              // 导航工具
        ICON_INFO,               // 导航关于

        // 状态图标 (24x24px)
        ICON_SUCCESS,            // 成功状态
        ICON_WARNING,            // 警告状态
        ICON_ERROR,              // 错误状态
        ICON_PROCESSING,         // 处理中状态

        // 媒体图标 (32x32px)
        ICON_VIDEO,              // 视频文件
        ICON_AUDIO,              // 音频文件
        ICON_SUBTITLE,           // 字幕文件
        ICON_DANMAKU,            // 弹幕文件

        // 其他
        ICON_CLEAR,              // 清空
        ICON_EXPORT,             // 导出
        ICON_REFRESH,            // 刷新
        ICON_CHECK,              // 检查
        ICON_QUESTION            // 帮助
    };

    /**
     * @brief 获取指定类型的图标
     * @param type 图标类型
     * @param size 图标尺寸
     * @return QIcon 图标对象
     */
    static QIcon getIcon(IconType type, int size = 32);

    /**
     * @brief 创建彩色圆形图标
     * @param emoji emoji字符
     * @param color 背景颜色
     * @param size 图标尺寸
     * @return QIcon 图标对象
     */
    static QIcon createEmojiIcon(const QString& emoji, const QColor& color, int size);

    /**
     * @brief 创建文字图标
     * @param text 文字
     * @param color 背景颜色
     * @param size 图标尺寸
     * @return QIcon 图标对象
     */
    static QIcon createTextIcon(const QString& text, const QColor& color, int size);

private:
    // 预定义颜色
    static const QColor PRIMARY_COLOR;
    static const QColor SUCCESS_COLOR;
    static const QColor WARNING_COLOR;
    static const QColor ERROR_COLOR;
    static const QColor SURFACE_COLOR;
    static const QColor TEXT_COLOR;

    // 图标映射函数
    static QIcon createPlayIcon(int size);
    static QIcon createFolderIcon(int size);
    static QIcon createSettingsIcon(int size);
    static QIcon createDirectoryIcon(int size);
    static QIcon createGearIcon(int size);
    static QIcon createToolsIcon(int size);
    static QIcon createInfoIcon(int size);
    static QIcon createSuccessIcon(int size);
    static QIcon createWarningIcon(int size);
    static QIcon createErrorIcon(int size);
    static QIcon createProcessingIcon(int size);
    static QIcon createVideoIcon(int size);
    static QIcon createAudioIcon(int size);
    static QIcon createSubtitleIcon(int size);
    static QIcon createDanmakuIcon(int size);
};

#endif // ICONMANAGER_H