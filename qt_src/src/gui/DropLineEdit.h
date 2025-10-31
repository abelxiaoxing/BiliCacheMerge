#ifndef DROPLINEEDIT_H
#define DROPLINEEDIT_H

#include <QLineEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QFont>

/**
 * @brief 支持文件拖拽的输入框组件
 *
 * 继承自QLineEdit，增加了文件和文件夹拖拽功能
 * 提供视觉反馈，显示拖拽状态
 */
class DropLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DropLineEdit(QWidget *parent = nullptr);

    void setDragActive(bool active);
    bool isDragActive() const { return m_isDragActive; }

signals:
    void fileDropped(const QString &filePath);
    void directoryDropped(const QString &directoryPath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateDragState(bool active);
    void drawDropHint(QPainter &painter);
    void updateDropHintGeometry();

    bool m_isDragActive;
    QRect m_dropHintRect;
    QString m_dropHintText;
    QFont m_hintFont;
    QColor m_hintColor;
    QColor m_activeBorderColor;
    QColor m_normalBorderColor;
};

#endif // DROPLINEEDIT_H