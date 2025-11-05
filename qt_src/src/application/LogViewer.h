#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QString>
#include <QPlainTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QApplication>
#include <QTimer>

class LogViewer : public QDialog
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer();

private slots:
    void onOpenLogClicked();
    void onRefreshClicked();
    void onClearClicked();
    void onExportClicked();
    void onAutoRefreshToggled(bool checked);

private:
    void loadLogFile(const QString &filePath);
    void createLogContent();
    QString getDefaultLogPath();
    bool openWithSystemApp(const QString &filePath);

    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QPlainTextEdit *m_logDisplay;
    QPushButton *m_openLogButton;
    QPushButton *m_refreshButton;
    QPushButton *m_clearButton;
    QPushButton *m_exportButton;
    QPushButton *m_closeButton;
    QCheckBox *m_autoRefreshCheck;
    QLabel *m_logPathLabel;
    QLabel *m_statusLabel;

    // 数据
    QString m_currentLogPath;
    QTimer *m_autoRefreshTimer;
};

#endif // LOGVIEWER_H
