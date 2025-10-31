#include "DropLineEdit.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>

DropLineEdit::DropLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_isDragActive(false)
{
    // 设置接受拖拽
    setAcceptDrops(true);

    // 设置样式
    setReadOnly(true);

    // 初始化字体
    m_hintFont = font();
    m_hintFont.setItalic(true);
    m_hintFont.setPointSize(font().pointSize() - 1);

    // 设置颜色
    m_hintColor = QColor(128, 128, 128);
    m_normalBorderColor = QColor(200, 200, 200);
    m_activeBorderColor = QColor(251, 114, 153); // B站粉色

    // 设置提示文本
    m_dropHintText = "拖拽文件或文件夹到此处";

    // 更新提示区域
    updateDropHintGeometry();

    // 设置默认样式
    setStyleSheet(QString(
        "DropLineEdit {"
        "    border: 2px dashed %1;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    background-color: white;"
        "}"
        "DropLineEdit:hover {"
        "    border-color: %2;"
        "}"
    ).arg(m_normalBorderColor.name(), QColor(220, 220, 220).name()));
}

void DropLineEdit::setDragActive(bool active)
{
    if (m_isDragActive != active) {
        m_isDragActive = active;
        updateDragState(active);
    }
}

void DropLineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        setDragActive(true);
    }
}

void DropLineEdit::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event)
    setDragActive(false);
}

void DropLineEdit::dropEvent(QDropEvent *event)
{
    setDragActive(false);

    const QMimeData *mimeData = event->mimeData();
    if (!mimeData->hasUrls()) {
        return;
    }

    QList<QUrl> urls = mimeData->urls();
    if (urls.isEmpty()) {
        return;
    }

    // 处理第一个URL（简化处理）
    QString path = urls.first().toLocalFile();
    if (path.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(path);
    QString displayText;

    if (fileInfo.isDir()) {
        // 处理文件夹
        displayText = fileInfo.absoluteFilePath();
        emit directoryDropped(displayText);
        qDebug() << "Directory dropped:" << displayText;
    } else {
        // 处理文件
        displayText = fileInfo.absoluteFilePath();
        emit fileDropped(displayText);
        qDebug() << "File dropped:" << displayText;
    }

    // 更新输入框内容
    setText(displayText);
    setCursorPosition(0); // 光标移到开头

    event->acceptProposedAction();
}

void DropLineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    // 如果没有文本且不在拖拽状态，不绘制自定义提示文本
    // 让Qt的placeholderText正常显示
    if (text().isEmpty() && !m_isDragActive) {
        // 不绘制自定义提示，避免与placeholderText重合
    }

    // 如果在拖拽状态，绘制拖拽提示
    if (m_isDragActive) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor(251, 114, 153, 20)); // 半透明粉色背景

        // 绘制拖拽提示文本
        painter.setPen(QPen(m_activeBorderColor, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);

        painter.setFont(m_hintFont);
        painter.setPen(m_activeBorderColor);
        painter.drawText(rect(), Qt::AlignCenter, "释放以添加文件/文件夹");
    }
}

void DropLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    updateDropHintGeometry();
}

void DropLineEdit::updateDragState(bool active)
{
    if (active) {
        setStyleSheet(QString(
            "DropLineEdit {"
            "    border: 2px solid %1;"
            "    border-radius: 8px;"
            "    padding: 8px;"
            "    background-color: rgba(251, 114, 153, 0.1);"
            "}"
        ).arg(m_activeBorderColor.name()));
    } else {
        setStyleSheet(QString(
            "DropLineEdit {"
            "    border: 2px dashed %1;"
            "    border-radius: 8px;"
            "    padding: 8px;"
            "    background-color: white;"
            "}"
            "DropLineEdit:hover {"
            "    border-color: %2;"
            "}"
        ).arg(m_normalBorderColor.name(), QColor(220, 220, 220).name()));
    }

    update();
}

void DropLineEdit::drawDropHint(QPainter &painter)
{
    painter.setFont(m_hintFont);
    painter.setPen(m_hintColor);

    QRect textRect = rect().adjusted(10, 0, -10, 0);
    Qt::Alignment alignment = Qt::AlignVCenter;

    if (isRightToLeft()) {
        alignment |= Qt::AlignRight;
    } else {
        alignment |= Qt::AlignLeft;
    }

    painter.drawText(textRect, alignment, m_dropHintText);
}

void DropLineEdit::updateDropHintGeometry()
{
    // 更新提示文本区域（如果需要的话）
    QFontMetrics metrics(m_hintFont);
    m_dropHintRect = metrics.boundingRect(rect(), Qt::AlignLeft | Qt::AlignVCenter, m_dropHintText);
}

