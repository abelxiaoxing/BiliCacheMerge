// 简单测试，验证entry.json和媒体文件的存在性
#include <iostream>
#include <fstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QFile>

int main() {
    // 测试entry.json是否存在和可读
    QFile entryFile("/home/abelxiaoxing/work/BiliCacheMerge/test_data/entry.json");
    if (!entryFile.open(QIODevice::ReadOnly)) {
        std::cout << "ERROR: Cannot open entry.json" << std::endl;
        return 1;
    }

    QByteArray data = entryFile.readAll();
    entryFile.close();

    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        std::cout << "ERROR: JSON parse error: " << parseError.errorString().toStdString() << std::endl;
        return 1;
    }

    if (!doc.isObject()) {
        std::cout << "ERROR: JSON is not an object" << std::endl;
        return 1;
    }

    QJsonObject jsonObj = doc.object();
    QString typeTag = jsonObj.value("type_tag").toString();
    QString title = jsonObj.value("title").toString();

    std::cout << "SUCCESS: entry.json parsed successfully" << std::endl;
    std::cout << "Type Tag: " << typeTag.toStdString() << std::endl;
    std::cout << "Title: " << title.toStdString().substr(0, 50) << "..." << std::endl;

    // 检查媒体文件是否存在
    QString videoPath = QString("/home/abelxiaoxing/work/BiliCacheMerge/test_data/%1/video.m4s").arg(typeTag);
    QString audioPath = QString("/home/abelxiaoxing/work/BiliCacheMerge/test_data/%1/audio.m4s").arg(typeTag);

    QFile videoFile(videoPath);
    QFile audioFile(audioPath);

    std::cout << "Video file: " << videoPath.toStdString() << " - "
              << (videoFile.exists() ? "EXISTS" : "MISSING") << std::endl;
    std::cout << "Audio file: " << audioPath.toStdString() << " - "
              << (audioFile.exists() ? "EXISTS" : "MISSING") << std::endl;

    return 0;
}