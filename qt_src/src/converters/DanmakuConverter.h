#ifndef DANMAKUCONVERTER_H
#define DANMAKUCONVERTER_H

#include <QObject>
#include <QString>

/**
 * @brief 弹幕转换器
 *
 * 将XML格式的弹幕文件转换为ASS字幕文件
 */
class DanmakuConverter : public QObject
{
    Q_OBJECT

public:
    explicit DanmakuConverter(QObject *parent = nullptr);

    // 转换方法
    bool convertXmlToAss(const QString &xmlPath, const QString &assPath);
    bool convertXmlToAss(const QString &xmlPath, const QString &assPath,
                        int fontSize, double textOpacity, double reverseBlank,
                        int durationMarquee, int durationStill,
                        bool reduceComments);

    // 配置方法
    void setFontSize(int size);
    void setTextOpacity(double opacity);
    void setReverseBlank(double blank);
    void setDurationMarquee(int duration);
    void setDurationStill(int duration);
    void setReduceComments(bool reduce);

signals:
    void conversionStarted(const QString &inputFile);
    void conversionProgress(int progress);
    void conversionFinished(const QString &outputFile, bool success);
    void conversionError(const QString &error);

private:
    QString generateAssHeader() const;
    QString processDanmakuElement(const QString &xmlElement) const;

    // 配置参数
    int m_fontSize;
    double m_textOpacity;
    double m_reverseBlank;
    int m_durationMarquee;
    int m_durationStill;
    bool m_reduceComments;
};

#endif // DANMAKUCONVERTER_H