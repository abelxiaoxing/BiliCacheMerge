#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "application/MainWindow.h"

/**
 * @brief BiliCacheMerge Qt C++ 版本主函数
 *
 * 程序启动流程:
 * 1. 创建QApplication实例
 * 2. 初始化应用程序设置
 * 3. 创建主窗口
 * 4. 启动事件循环
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Qt B站缓存合并工具");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("BiliCacheMerge");

    // 设置应用程序样式
    app.setStyle("Fusion");

    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();

    // 如果提供了命令行参数，自动设置目录
    if (argc > 1) {
        QString dirPath = QString::fromLocal8Bit(argv[1]);
        mainWindow.setDirectoryPath(dirPath);
    }

    // 记录启动日志
    qDebug() << "=== Qt B站缓存合并工具 2.0.0 Started ===";
    qDebug() << "主窗口初始化完成";

    return app.exec();
}