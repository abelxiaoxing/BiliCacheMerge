#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include "src/core/ConfigManager.h"
#include "src/core/PatternManager.h"
#include "src/core/FileScanner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 创建配置管理器和模式管理器
    ConfigManager configManager;
    PatternManager patternManager(&configManager);

    // 加载所有模式
    if (!patternManager.loadAllPatterns()) {
        qDebug() << "Failed to load patterns";
        return 1;
    }

    qDebug() << "Loaded patterns:" << patternManager.patternNames();

    // 创建文件扫描器
    FileScanner scanner(&configManager, &patternManager);

    // 配置扫描
    FileScanner::ScanConfig scanConfig;
    scanConfig.searchPath = "/home/abelxiaoxing/work/BiliCacheMerge/test_data";
    scanConfig.patternName = "Android";  // 使用Android模式

    // 执行扫描
    bool success = scanner.scan(scanConfig);

    qDebug() << "Scan success:" << success;
    qDebug() << "Total files found:" << scanner.totalFiles();
    qDebug() << "Total groups found:" << scanner.totalGroups();

    // 输出找到的视频组
    QList<FileScanner::VideoGroup> groups = scanner.videoGroups();
    for (const auto &group : groups) {
        qDebug() << "Group pattern:" << group.patternName;
        qDebug() << "Group entry:" << group.groupEntryPath;
        qDebug() << "Files in group:" << group.files.size();
        for (const auto &file : group.files) {
            qDebug() << "  - Entry:" << file.entryPath;
            qDebug() << "    Video:" << file.videoPath;
            qDebug() << "    Audio:" << file.audioPath;
            qDebug() << "    Danmu:" << file.danmuPath;
        }
    }

    return 0;
}