#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QDebug>

QString resolvePathTemplate(const QString &templateStr, const QString &entryPath) {
    if (templateStr.isEmpty() || templateStr == "null") {
        return QString();
    }

    QString result = templateStr;

    // 首先处理路径分隔符（在变量替换之前）
    if (QDir::separator() == '/') {
        result.replace('\\', '/');
    } else {
        result.replace('/', '\\');
    }

    // 获取entry文件的目录
    QFileInfo entryInfo(entryPath);
    QString entryDir = entryInfo.absolutePath();
    QString entryBaseName = entryInfo.baseName();

    // 处理%group%和%episode%变量
    result.replace("%group%", entryDir.split('/').last());
    result.replace("%episode%", entryBaseName);

    // 处理其他JSON字段变量（包括嵌套字段）
    QFile entryFile(entryPath);
    if (entryFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = entryFile.readAll();
        entryFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject jsonObj = doc.object();

            // 预先提取所有可能的变量值
            QMap<QString, QString> variables;

            // 处理根级别字段
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                QString key = it.key();
                QVariant value = it.value().toVariant();
                if (value.isValid()) {
                    variables["%" + key + "%"] = value.toString();
                }
            }

            // 处理常见的嵌套字段（如page_data-cid）
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                if (it.value().isObject()) {
                    QJsonObject subObj = it.value().toObject();
                    for (auto subIt = subObj.begin(); subIt != subObj.end(); ++subIt) {
                        QString nestedKey = it.key() + "-" + subIt.key();
                        QVariant value = subIt.value().toVariant();
                        if (value.isValid()) {
                            variables["%" + nestedKey + "%"] = value.toString();
                        }
                    }
                }
            }

            // 执行变量替换
            for (auto varIt = variables.begin(); varIt != variables.end(); ++varIt) {
                result.replace(varIt.key(), varIt.value());
            }
        }
    }

    // 如果结果是相对路径，需要转换为绝对路径
    if (!QDir::isAbsolutePath(result)) {
        result = entryDir + "/" + result;
    }

    return result;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString entryPath = "/home/abelxiaoxing/work/BiliCacheMerge/test_data/entry.json";
    QString videoTemplate = "%type_tag%/video.m4s";
    QString audioTemplate = "%type_tag%/audio.m4s";

    qDebug() << "Entry path:" << entryPath;
    qDebug() << "Video template:" << videoTemplate;
    qDebug() << "Audio template:" << audioTemplate;

    QString resolvedVideo = resolvePathTemplate(videoTemplate, entryPath);
    QString resolvedAudio = resolvePathTemplate(audioTemplate, entryPath);

    qDebug() << "Resolved video path:" << resolvedVideo;
    qDebug() << "Resolved audio path:" << resolvedAudio;

    // 检查文件是否存在
    qDebug() << "Video file exists:" << QFile::exists(resolvedVideo);
    qDebug() << "Audio file exists:" << QFile::exists(resolvedAudio);

    return 0;
}