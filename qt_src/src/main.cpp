#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>

#include "gui/MainWindow.h"
#include "core/ConfigManager.h"
#include "utils/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Qt B站缓存合并工具");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtBiliMerge");
    app.setOrganizationDomain("qtbili.merge");

    // Qt6中高DPI支持已默认启用，这些属性已被弃用

    // 初始化配置管理器
    ConfigManager::getInstance();

    // 初始化日志系统
    Logger::getInstance();

    // 设置中文翻译（如果需要）
    QTranslator translator;
    if (translator.load(QLocale(), "qt_", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&translator);
    }

    try {
        // 创建主窗口
        MainWindow window;
        window.show();

        // 运行应用程序
        return app.exec();

    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "启动错误",
                             QString("应用程序启动失败：\n%1").arg(e.what()));
        return -1;
    } catch (...) {
        QMessageBox::critical(nullptr, "启动错误",
                             "应用程序发生未知错误，启动失败。");
        return -1;
    }
}