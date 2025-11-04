#include "IconManager.h"
#include <QStyle>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QtMath>

// 颜色定义
const QColor IconManager::PRIMARY_COLOR     = QColor(33, 150, 243);   // 蓝色
const QColor IconManager::SUCCESS_COLOR     = QColor(76, 175, 80);    // 绿色
const QColor IconManager::WARNING_COLOR     = QColor(255, 152, 0);    // 橙色
const QColor IconManager::ERROR_COLOR       = QColor(244, 67, 54);    // 红色
const QColor IconManager::SURFACE_COLOR     = QColor(255, 255, 255);  // 白色
const QColor IconManager::TEXT_COLOR        = QColor(255, 255, 255);  // 白色文字

QIcon IconManager::getIcon(IconType type, int size)
{
    switch (type) {
        case ICON_PLAY:
            return createPlayIcon(size);
        case ICON_FOLDER:
            return createFolderIcon(size);
        case ICON_SETTINGS:
            return createSettingsIcon(size);
        case ICON_DIRECTORY:
            return createDirectoryIcon(size);
        case ICON_GEAR:
            return createGearIcon(size);
        case ICON_TOOLS:
            return createToolsIcon(size);
        case ICON_INFO:
            return createInfoIcon(size);
        case ICON_SUCCESS:
            return createSuccessIcon(size);
        case ICON_WARNING:
            return createWarningIcon(size);
        case ICON_ERROR:
            return createErrorIcon(size);
        case ICON_PROCESSING:
            return createProcessingIcon(size);
        case ICON_VIDEO:
            return createVideoIcon(size);
        case ICON_AUDIO:
            return createAudioIcon(size);
        case ICON_SUBTITLE:
            return createSubtitleIcon(size);
        case ICON_DANMAKU:
            return createDanmakuIcon(size);
        default:
            // 如果没有对应图标，使用Qt内置图标
            return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }
}

QIcon IconManager::createEmojiIcon(const QString& emoji, const QColor& color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形背景
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // 绘制emoji
    QFont font = painter.font();
    font.setPointSizeF(size * 0.5);
    painter.setFont(font);
    painter.setPen(TEXT_COLOR);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, emoji);

    return QIcon(pixmap);
}

QIcon IconManager::createTextIcon(const QString& text, const QColor& color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形背景
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // 绘制文字
    QFont font = painter.font();
    font.setPointSizeF(size * 0.3);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(TEXT_COLOR);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return QIcon(pixmap);
}

// 主要操作图标实现
QIcon IconManager::createPlayIcon(int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形背景
    painter.setBrush(PRIMARY_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // 绘制播放三角形
    painter.setBrush(TEXT_COLOR);
    QPolygonF triangle;
    triangle << QPointF(size * 0.35, size * 0.25)
              << QPointF(size * 0.35, size * 0.75)
              << QPointF(size * 0.75, size * 0.5);
    painter.drawPolygon(triangle);

    return QIcon(pixmap);
}

QIcon IconManager::createFolderIcon(int size)
{
    return createEmojiIcon("📁", QColor(255, 152, 0), size);
}

QIcon IconManager::createSettingsIcon(int size)
{
    return createEmojiIcon("⚙️", QColor(97, 97, 97), size);
}

// 导航图标实现
QIcon IconManager::createDirectoryIcon(int size)
{
    return createEmojiIcon("📁", QColor(255, 152, 0), size);
}

QIcon IconManager::createGearIcon(int size)
{
    return createEmojiIcon("⚙️", QColor(97, 97, 97), size);
}

QIcon IconManager::createToolsIcon(int size)
{
    return createEmojiIcon("🛠️", QColor(0, 150, 136), size);
}

QIcon IconManager::createInfoIcon(int size)
{
    return createEmojiIcon("ℹ️", PRIMARY_COLOR, size);
}

// 状态图标实现
QIcon IconManager::createSuccessIcon(int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形背景
    painter.setBrush(SUCCESS_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // 绘制对勾
    painter.setBrush(Qt::NoBrush);
    QPen pen(TEXT_COLOR);
    pen.setWidthF(size * 0.08);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    // 绘制对勾形状
    QPointF points[3] = {
        QPointF(size * 0.25, size * 0.5),
        QPointF(size * 0.4, size * 0.65),
        QPointF(size * 0.75, size * 0.35)
    };
    painter.drawPolyline(points, 3);

    return QIcon(pixmap);
}

QIcon IconManager::createWarningIcon(int size)
{
    return createEmojiIcon("⚠️", WARNING_COLOR, size);
}

QIcon IconManager::createErrorIcon(int size)
{
    return createEmojiIcon("❌", ERROR_COLOR, size);
}

QIcon IconManager::createProcessingIcon(int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形背景
    painter.setBrush(PRIMARY_COLOR);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // 绘制旋转效果（使用简单的线条表示）
    painter.setPen(QPen(TEXT_COLOR, size * 0.08, Qt::SolidLine, Qt::RoundCap));

    // 绘制旋转箭头
    int centerX = size / 2;
    int centerY = size / 2;
    int radius = size * 0.3;

    // 绘制弧线
    int startAngle = 30 * 16;
    int spanAngle = 300 * 16;
    painter.drawArc(centerX - radius, centerY - radius, radius * 2, radius * 2, startAngle, spanAngle);

    return QIcon(pixmap);
}

// 媒体图标实现
QIcon IconManager::createVideoIcon(int size)
{
    return createEmojiIcon("🎬", QColor(156, 39, 176), size);
}

QIcon IconManager::createAudioIcon(int size)
{
    return createEmojiIcon("🎵", QColor(255, 193, 7), size);
}

QIcon IconManager::createSubtitleIcon(int size)
{
    return createTextIcon("CC", QColor(0, 188, 212), size);
}

QIcon IconManager::createDanmakuIcon(int size)
{
    return createEmojiIcon("💬", QColor(233, 30, 99), size);
}