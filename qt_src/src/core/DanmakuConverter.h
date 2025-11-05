#ifndef DANMAKUCONVERTER_H
#define DANMAKUCONVERTER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QRect>

class QXmlStreamReader;

struct DanmakuItem {
    double time;          // 时间（秒）
    int position;         // 位置
    int index;           // 索引
    QString text;        // 弹幕文本
    QString type;        // 类型：滚动(0,2,3,4)、定位(bilipos)
    int color;           // 颜色
    double fontSize;     // 字体大小
    double lineHeight;   // 行高
    double textLength;   // 文本长度
};

struct DanmakuConfig {
    int fontSize;            // 基础字体大小
    double textOpacity;      // 文字透明度 (0.0-1.0)
    double durationMarquee;  // 滚动弹幕持续时间（秒）
    double durationStill;    // 静止弹幕持续时间（秒）
    double reverseBlank;     // 反向空白
    bool reduceComments;     // 减少评论
    int stageWidth;          // 视频宽度
    int stageHeight;         // 视频高度
    QString fontFace;        // 字体名称
};

class DanmakuConverter : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuConverter(QObject *parent = nullptr);

    // 核心转换方法
    bool convertToASS(const QString &inputXmlPath, const QString &outputAssPath,
                     const DanmakuConfig &config);

    // 检测弹幕格式
    QString detectFormat(const QString &xmlPath);

signals:
    void conversionProgress(int percent);
    void conversionLog(const QString &message);

private:
    // XML解析
    void parseBilibiliXML(QXmlStreamReader &reader, QList<DanmakuItem> &items);
    void parseBilibili2XML(QXmlStreamReader &reader, QList<DanmakuItem> &items);

    // ASS生成
    void generateASS(const QList<DanmakuItem> &items, QTextStream &output,
                    const DanmakuConfig &config);
    void writeASSHeader(QTextStream &output, const DanmakuConfig &config, const QString &styleId);
    void writeComments(const QList<DanmakuItem> &items, QTextStream &output,
                      const DanmakuConfig &config, const QString &styleId);

    // 滚动弹幕处理
    void writeMovingComment(QTextStream &output, const DanmakuItem &item,
                           int row, const DanmakuConfig &config, const QString &styleId);
    void writeStillComment(QTextStream &output, const DanmakuItem &item,
                          int row, const DanmakuConfig &config, const QString &styleId);

    // 定位弹幕处理
    void writePositionedComment(QTextStream &output, const DanmakuItem &item,
                               const DanmakuConfig &config, const QString &styleId);

    // 碰撞检测
    int findAvailableRow(const QList<DanmakuItem> &items, const DanmakuItem &current,
                        const QList<QList<DanmakuItem>> &rows,
                        const DanmakuConfig &config);
    bool isRowFree(const QList<QList<DanmakuItem>> &rows, int lane, int startRow, int requiredRows,
                  double currentTime, const DanmakuConfig &config);

    // 工具函数
    QString convertTimestamp(double timestamp);
    QString convertColor(int rgb);
    QString assEscape(const QString &text);
    int calculateTextLength(const QString &text);
    QRect getZoomFactor(const QRect &source, const QRect &target);
    QPointF convertFlashRotation(int rotY, int rotZ, const QPointF &pos, const QRect &stage);
    QVariantMap safeListToMap(const QVariantList &list);

    // 解析位置弹幕参数
    QVariantMap parsePositionedArgs(const QString &jsonStr);

    // 减少评论功能
    bool shouldReduceComment(const DanmakuItem &current, const QList<DanmakuItem> &recentList);
    void filterDuplicates(QList<DanmakuItem> &items);
};

#endif // DANMAKUCONVERTER_H
