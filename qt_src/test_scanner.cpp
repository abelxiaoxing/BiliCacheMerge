#include <QCoreApplication>
#include <QDebug>
#include "src/core/ConfigManager.h"
#include "src/core/PatternManager.h"
#include "src/core/FileScanner.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        qDebug() << "Usage: test_scanner <directory>";
        return 1;
    }

    QString testDir = QString::fromLocal8Bit(argv[1]);
    qDebug() << "测试目录:" << testDir;

    // 创建核心组件
    ConfigManager* configManager = new ConfigManager();
    PatternManager* patternManager = new PatternManager(configManager);
    FileScanner* fileScanner = new FileScanner(configManager, patternManager);

    // 配置扫描参数
    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = testDir;
    scanConfig.patternName = QString(); // 使用所有可用模式
    scanConfig.oneDir = false;
    scanConfig.overwrite = false;
    scanConfig.danmuEnabled = true;
    scanConfig.coverEnabled = true;
    scanConfig.subtitleEnabled = false;
    scanConfig.ordered = true;

    // 连接信号
    QObject::connect(fileScanner, &FileScanner::scanLog, [](const QString& message) {
        qDebug() << "[扫描日志]" << message;
    });

    QObject::connect(fileScanner, &FileScanner::scanError, [](const QString& error) {
        qDebug() << "[扫描错误]" << error;
    });

    QObject::connect(fileScanner, &FileScanner::scanCompleted, [fileScanner](bool success) {
        if (success) {
            qDebug() << "扫描成功完成！";
            qDebug() << "找到" << fileScanner->totalGroups() << "个视频组，"
                     << fileScanner->totalFiles() << "个文件";

            // 输出视频组信息
            QList<FileScanner::VideoGroup> groups = fileScanner->videoGroups();
            for (int i = 0; i < groups.size(); ++i) {
                const FileScanner::VideoGroup& group = groups[i];
                qDebug() << "视频组" << (i+1) << ":" << group.patternName;
                qDebug() << "  组文件:" << group.groupEntryPath;
                qDebug() << "  文件数量:" << group.files.size();

                for (int j = 0; j < group.files.size(); ++j) {
                    const FileScanner::VideoFile& file = group.files[j];
                    qDebug() << "  文件" << (j+1) << ":" << file.entryPath;
                    qDebug() << "    视频:" << file.videoPath;
                    qDebug() << "    音频:" << file.audioPath;
                    qDebug() << "    标题:" << file.metadata.value("title", "unknown").toString();
                }
            }
        } else {
            qDebug() << "扫描失败！";
        }
        QCoreApplication::quit();
    });

    // 开始扫描
    qDebug() << "开始扫描...";
    fileScanner->scan(scanConfig);

    return app.exec();
}