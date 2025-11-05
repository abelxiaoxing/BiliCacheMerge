#include "DanmakuConverter.h"
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <cmath>

DanmakuConverter::DanmakuConverter(QObject *parent)
    : QObject(parent)
{
}

bool DanmakuConverter::convertToASS(const QString &inputXmlPath, const QString &outputAssPath,
                                    const DanmakuConfig &config)
{
    emit conversionLog("开始转换弹幕文件...");

    QFile inputFile(inputXmlPath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit conversionLog(QString("错误：无法打开文件 %1").arg(inputXmlPath));
        return false;
    }

    // 读取XML内容并解析
    QList<DanmakuItem> items;
    QXmlStreamReader reader(&inputFile);

    QString format = detectFormat(inputXmlPath);
    emit conversionLog(QString("检测到弹幕格式: %1").arg(format));

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == "i") {
                if (format == "Bilibili") {
                    parseBilibiliXML(reader, items);
                } else if (format == "Bilibili2") {
                    parseBilibili2XML(reader, items);
                }
                break; // 找到弹幕内容即可
            }
        }
    }

    inputFile.close();

    if (items.isEmpty()) {
        emit conversionLog("错误：未发现有效弹幕数据");
        return false;
    }

    emit conversionLog(QString("成功解析 %1 条弹幕").arg(items.size()));

    // 生成ASS文件
    QFile outputFile(outputAssPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit conversionLog(QString("错误：无法创建输出文件 %1").arg(outputAssPath));
        return false;
    }

    QTextStream output(&outputFile);
    output.setEncoding(QStringConverter::Utf8);

    QString styleId = QString("Danmaku2ASS_%1").arg(QTime::currentTime().msec(), 4, 16, QChar('0'));
    writeASSHeader(output, config, styleId);
    writeComments(items, output, config, styleId);

    outputFile.close();

    emit conversionLog(QString("转换完成：%1").arg(outputAssPath));
    return true;
}

QString DanmakuConverter::detectFormat(const QString &xmlPath)
{
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Unknown";
    }

    QByteArray header = file.read(50);
    file.close();

    if (header.startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\"?><i")) {
        return "Bilibili";
    } else if (header.startsWith("<?xml version=\"2.0\" encoding=\"UTF-8\"?><i")) {
        return "Bilibili2";
    }

    return "Unknown";
}

void DanmakuConverter::parseBilibiliXML(QXmlStreamReader &reader, QList<DanmakuItem> &items)
{
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::EndElement && reader.name() == "i") {
            break;
        }

        if (token == QXmlStreamReader::StartElement && reader.name() == "d") {
            QString p = reader.attributes().value("p").toString();

            // 格式: time,type,fontsize,color,mid,date,hash,mid,ctime(d)
            QStringList parts = p.split(',');

            if (parts.size() >= 5) {
                DanmakuItem item;
                item.time = parts[0].toDouble();
                item.type = parts[1];
                item.color = parts[3].toInt();
                item.index = items.size();
                item.text = reader.readElementText().replace("/n", "\n");

                // 根据类型设置参数
                if (item.type == "1" || item.type == "4" || item.type == "5" || item.type == "6") {
                    item.fontSize = parts[2].toDouble() * 25.0 / 25.0; // 基础字体大小
                    item.lineHeight = (item.text.count('\n') + 1) * item.fontSize;
                    item.textLength = calculateTextLength(item.text) * item.fontSize;
                    item.position = parts[4].toInt();

                    // 映射类型到轨道
                    static QMap<QString, int> typeMap = {{"1", 0}, {"4", 2}, {"5", 1}, {"6", 3}};
                    item.type = QString::number(typeMap.value(item.type, 0));
                } else if (item.type == "7") {
                    item.fontSize = parts[2].toDouble();
                    item.type = "bilipos";
                    item.position = parts[4].toInt();
                } else if (item.type == "8") {
                    continue; // 忽略脚本弹幕
                }

                items.append(item);
            }
        }
    }
}

void DanmakuConverter::parseBilibili2XML(QXmlStreamReader &reader, QList<DanmakuItem> &items)
{
    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::EndElement && reader.name() == "i") {
            break;
        }

        if (token == QXmlStreamReader::StartElement && reader.name() == "d") {
            QString p = reader.attributes().value("p").toString();

            // 格式: mode,fontsize,time,type,color,date,hash,mid,ctime,weight,dmode,pool,attr
            QStringList parts = p.split(',');

            if (parts.size() >= 7) {
                DanmakuItem item;
                item.time = parts[2].toDouble() / 1000.0; // 转换为秒
                item.type = parts[3];
                item.color = parts[4].toInt();
                item.fontSize = parts[1].toDouble();
                item.index = items.size();
                item.text = reader.readElementText().replace("/n", "\n");

                // 根据类型设置参数
                if (item.type == "1" || item.type == "4" || item.type == "5" || item.type == "6") {
                    item.lineHeight = (item.text.count('\n') + 1) * item.fontSize;
                    item.textLength = calculateTextLength(item.text) * item.fontSize;
                    item.position = parts[6].toInt();

                    static QMap<QString, int> typeMap = {{"1", 0}, {"4", 2}, {"5", 1}, {"6", 3}};
                    item.type = QString::number(typeMap.value(item.type, 0));
                } else if (item.type == "7") {
                    item.type = "bilipos";
                    item.position = parts[6].toInt();
                } else if (item.type == "8") {
                    continue; // 忽略脚本弹幕
                }

                items.append(item);
            }
        }
    }
}

void DanmakuConverter::writeASSHeader(QTextStream &output, const DanmakuConfig &config, const QString &styleId)
{
    double outline = std::max(config.fontSize / 25.0, 1.0);
    int alpha = 255 - static_cast<int>(config.textOpacity * 255);

    output << "[Script Info]\n";
    output << "; Script generated by BiliCacheMerge DanmakuConverter\n";
    output << "ScriptType: v4.00+\n";
    output << QString("PlayResX: %1\n").arg(config.stageWidth);
    output << QString("PlayResY: %1\n").arg(config.stageHeight);
    output << "Aspect Ratio: " << config.stageWidth << ":" << config.stageHeight << "\n";
    output << "Collisions: Normal\n";
    output << "WrapStyle: 2\n";
    output << "ScaledBorderAndShadow: yes\n";
    output << "YCbCr Matrix: TV.601\n\n";

    output << "[V4+ Styles]\n";
    output << "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, "
           << "OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, "
           << "ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, "
           << "Alignment, MarginL, MarginR, MarginV, Encoding\n";

    output << QString("Style: %1, %2, %3.0f, &H%4%5, &H%4%5, &H%6%7, &H%6%7, 0, 0, 0, 0, 100, 100, 0.00, 0.00, 1, %8.0f, 0, 7, 0, 0, 0, 0\n")
              .arg(styleId)
              .arg(config.fontFace)
              .arg(static_cast<double>(config.fontSize), 0, 'f', 0)
              .arg(alpha, 2, 16, QChar('0'))
              .arg("FFFFFF")
              .arg("00")
              .arg("000000")
              .arg(outline, 0, 'f', 0);

    output << "\n[Events]\n";
    output << "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";
}

void DanmakuConverter::writeComments(const QList<DanmakuItem> &items, QTextStream &output,
                                     const DanmakuConfig &config, const QString &styleId)
{
    QList<QList<DanmakuItem>> rows(4); // 4个滚动轨道
    int bottomReserved = static_cast<int>(config.stageHeight * config.reverseBlank);

    emit conversionLog("开始处理弹幕...");

    for (int i = 0; i < items.size(); ++i) {
        if (i % 100 == 0) {
            emit conversionProgress(static_cast<int>((i * 100.0) / items.size()));
            QCoreApplication::processEvents();
        }

        const DanmakuItem &item = items[i];

        // 滚动弹幕
        if (item.type != "bilipos") {
            int row = findAvailableRow(items, item, rows, config);
            if (row >= 0) {
                // 检查轨道类型
                bool isTop = (item.type.toInt() == 1 || item.type.toInt() == 2);
                if (isTop && row < (config.stageHeight - bottomReserved) / item.lineHeight) {
                    writeStillComment(output, item, row, config, styleId);
                } else if (!isTop) {
                    writeMovingComment(output, item, row, config, styleId);
                }

                // 标记占用行
                int rowEnd = row + static_cast<int>(std::ceil(item.lineHeight));
                for (int r = row; r < rowEnd && r < rows[item.type.toInt()].size(); ++r) {
                    rows[item.type.toInt()][r] = item;
                }
            }
        }
        // 定位弹幕
        else {
            writePositionedComment(output, item, config, styleId);
        }
    }

    emit conversionProgress(100);
}

void DanmakuConverter::writeMovingComment(QTextStream &output, const DanmakuItem &item,
                                          int row, const DanmakuConfig &config, const QString &styleId)
{
    QString text = assEscape(item.text);
    QString styles;

    // 判断滚动方向：顶部滚动 vs 底部滚动
    bool isBottom = (item.type.toInt() == 3); // 底部滚动
    if (isBottom) {
        styles = QString("\\move(%1, %2, %3, %2)")
                 .arg(-static_cast<int>(item.textLength))
                 .arg(row)
                 .arg(config.stageWidth);
    } else {
        styles = QString("\\move(%1, %2, %3, %2)")
                 .arg(config.stageWidth)
                 .arg(row)
                 .arg(-static_cast<int>(item.textLength));
    }

    // 字体大小
    if (std::abs(item.fontSize - 25.0) > 1) {
        styles += QString("\\fs%1").arg(item.fontSize, 0, 'f', 0);
    }

    // 颜色
    if (item.color != 0xffffff) {
        styles += QString("\\c&H%1&").arg(convertColor(item.color));
        if (item.color == 0x000000) {
            styles += "\\3c&HFFFFFF&";
        }
    }

    QString startTime = convertTimestamp(item.time);
    QString endTime = convertTimestamp(item.time + config.durationMarquee);

    output << QString("Dialogue: 2,%1,%2,%3,,0000,0000,0000,,{%4}%5\n")
              .arg(startTime, endTime, styleId, styles, text);
}

void DanmakuConverter::writeStillComment(QTextStream &output, const DanmakuItem &item,
                                         int row, const DanmakuConfig &config, const QString &styleId)
{
    QString text = assEscape(item.text);
    QString styles;

    // 顶部固定 vs 底部固定
    bool isTop = (item.type.toInt() == 1 || item.type.toInt() == 2);
    if (isTop) {
        styles = QString("\\an8\\pos(%1, %2)")
                 .arg(config.stageWidth / 2)
                 .arg(row);
    } else {
        styles = QString("\\an2\\pos(%1, %2)")
                 .arg(config.stageWidth / 2)
                 .arg(config.stageHeight - row);
    }

    // 字体大小
    if (std::abs(item.fontSize - 25.0) > 1) {
        styles += QString("\\fs%1").arg(item.fontSize, 0, 'f', 0);
    }

    // 颜色
    if (item.color != 0xffffff) {
        styles += QString("\\c&H%1&").arg(convertColor(item.color));
        if (item.color == 0x000000) {
            styles += "\\3c&HFFFFFF&";
        }
    }

    QString startTime = convertTimestamp(item.time);
    QString endTime = convertTimestamp(item.time + config.durationStill);

    output << QString("Dialogue: 2,%1,%2,%3,,0000,0000,0000,,{%4}%5\n")
              .arg(startTime, endTime, styleId, styles, text);
}

void DanmakuConverter::writePositionedComment(QTextStream &output, const DanmakuItem &item,
                                              const DanmakuConfig &config, const QString &styleId)
{
    QVariantMap args = parsePositionedArgs(item.text);

    if (args.isEmpty()) {
        return; // 参数无效
    }

    QString text = assEscape(args.value("text", "").toString());

    // 位置参数
    double fromX = args.value("from_x", 0).toDouble();
    double fromY = args.value("from_y", 0).toDouble();
    double toX = args.value("to_x", fromX).toDouble();
    double toY = args.value("to_y", fromY).toDouble();

    // 透明度
    QStringList alphaList = args.value("alpha", "1").toString().split('-');
    double fromAlpha = alphaList.value(0, "1").toDouble();
    double toAlpha = alphaList.value(1, QString::number(fromAlpha)).toDouble();

    fromAlpha = 255 - static_cast<int>(fromAlpha * 255);
    toAlpha = 255 - static_cast<int>(toAlpha * 255);

    // 旋转
    int rotY = args.value("rotate_y", 0).toInt();
    int rotZ = args.value("rotate_z", 0).toInt();

    // 持续时间
    double lifetime = args.value("lifetime", 4500).toDouble() / 1000.0;
    double duration = args.value("duration", lifetime * 1000).toDouble() / 1000.0;
    int delay = args.value("delay", 0).toInt();

    QString styles;

    // 旋转和位置
    QPointF fromPos(fromX, fromY);
    QPointF toPos(toX, toY);
    // 这里简化处理，实际需要3D坐标转换
    styles += QString("\\pos(%1, %2)").arg(fromPos.x()).arg(fromPos.y());

    // 字体
    QString fontFace = args.value("fontface").toString();
    if (!fontFace.isEmpty()) {
        styles += QString("\\fn%1").arg(assEscape(fontFace));
    }

    // 字体大小
    styles += QString("\\fs%1").arg(item.fontSize, 0, 'f', 0);

    // 颜色
    if (item.color != 0xffffff) {
        styles += QString("\\c&H%1&").arg(convertColor(item.color));
    }

    // 透明度动画
    if (fromAlpha == toAlpha) {
        styles += QString("\\alpha&H%1").arg(static_cast<int>(fromAlpha), 2, 16, QChar('0'));
    } else {
        styles += QString("\\fade(%1, %2, %2, 0, %3, %3, %3)")
                  .arg(fromAlpha).arg(toAlpha).arg(lifetime * 1000);
    }

    QString startTime = convertTimestamp(item.time);
    QString endTime = convertTimestamp(item.time + lifetime);

    output << QString("Dialogue: -1,%1,%2,%3,,0000,0000,0000,,{%4}%5\n")
              .arg(startTime, endTime, styleId, styles, text);
}

int DanmakuConverter::findAvailableRow(const QList<DanmakuItem> &items, const DanmakuItem &current,
                                       const QList<QList<DanmakuItem>> &rows,
                                       const DanmakuConfig &config)
{
    int lane = current.type.toInt();
    int bottomReserved = static_cast<int>(config.stageHeight * config.reverseBlank);

    for (int row = 0; row < rows[lane].size(); ++row) {
        if (isRowFree(rows, lane, row, static_cast<int>(std::ceil(current.lineHeight)),
                     current.time, config)) {
            return row;
        }
    }

    return -1;
}

bool DanmakuConverter::isRowFree(const QList<QList<DanmakuItem>> &rows, int lane, int startRow,
                                int requiredRows, double currentTime,
                                const DanmakuConfig &config)
{
    for (int i = startRow; i < startRow + requiredRows; ++i) {
        if (i >= rows[lane].size()) {
            return true;
        }

        const DanmakuItem &existing = rows[lane][i];
        if (existing.time > 0) {
            // 简化的碰撞检测
            double thresholdTime = currentTime - config.durationMarquee *
                                  (1.0 - config.stageWidth / (existing.textLength + config.stageWidth));

            if (existing.time > thresholdTime) {
                return false;
            }
        }
    }

    return true;
}

QString DanmakuConverter::convertTimestamp(double timestamp)
{
    timestamp = std::round(timestamp * 100.0);

    int hour = static_cast<int>(timestamp / 360000);
    timestamp -= hour * 360000;

    int minute = static_cast<int>(timestamp / 6000);
    timestamp -= minute * 6000;

    int second = static_cast<int>(timestamp / 100);
    int centisecond = static_cast<int>(timestamp) % 100;

    return QString("%1:%2:%3.%4")
           .arg(hour).arg(minute, 2, 10, QChar('0'))
           .arg(second, 2, 10, QChar('0'))
           .arg(centisecond, 2, 10, QChar('0'));
}

QString DanmakuConverter::convertColor(int rgb)
{
    if (rgb == 0x000000) {
        return "000000";
    } else if (rgb == 0xffffff) {
        return "FFFFFF";
    }

    int r = (rgb >> 16) & 0xff;
    int g = (rgb >> 8) & 0xff;
    int b = rgb & 0xff;

    // BGR格式 (ASS使用BGR)
    return QString("%1%2%3")
           .arg(b, 2, 16, QChar('0'))
           .arg(g, 2, 16, QChar('0'))
           .arg(r, 2, 16, QChar('0'));
}

QString DanmakuConverter::assEscape(const QString &text)
{
    QString result = text;
    result.replace('\\', "\\\\");
    result.replace('{', "\\{");
    result.replace('}', "\\}");
    result.replace('\n', "\\N");

    // 处理行首空格
    QStringList lines = result.split("\\N");
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        int leadingSpaces = 0;
        while (leadingSpaces < line.length() && line[leadingSpaces] == ' ') {
            leadingSpaces++;
        }

        if (leadingSpaces > 0 && leadingSpaces < line.length()) {
            // 使用全角空格替换前导空格
            line = QString(QChar(0x2007)).repeated(leadingSpaces) + line.mid(leadingSpaces);
        } else if (leadingSpaces == line.length()) {
            line = QString(QChar(0x2007)).repeated(leadingSpaces);
        }

        lines[i] = line;
    }

    return lines.join("\\N");
}

int DanmakuConverter::calculateTextLength(const QString &text)
{
    int maxLen = 0;
    QStringList lines = text.split('\n');
    for (const QString &line : lines) {
        if (line.length() > maxLen) {
            maxLen = line.length();
        }
    }
    return maxLen;
}

QVariantMap DanmakuConverter::parsePositionedArgs(const QString &jsonStr)
{
    QVariantMap args;

    // 尝试解析JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();

        // 提取参数
        args["text"] = obj.value("text").toString();
        args["from_x"] = obj.value("x").toDouble();
        args["from_y"] = obj.value("y").toDouble();
        args["to_x"] = obj.value("to_x").toDouble();
        args["to_y"] = obj.value("to_y").toDouble();
        args["alpha"] = obj.value("alpha").toString("1");
        args["rotate_y"] = obj.value("rotate_y").toInt();
        args["rotate_z"] = obj.value("rotate_z").toInt();
        args["lifetime"] = obj.value("lifetime").toDouble(4500);
        args["duration"] = obj.value("duration").toDouble(4500);
        args["delay"] = obj.value("delay").toInt();
        args["fontface"] = obj.value("font").toString();
    } else {
        // 简单格式解析
        args["text"] = jsonStr;
        args["from_x"] = 0;
        args["from_y"] = 0;
    }

    return args;
}
