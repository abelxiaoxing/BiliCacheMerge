#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QScrollArea>
#include <QLabel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QApplication>
#include <QScreen>

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(const QString &defaultTopic = QString(), QWidget *parent = nullptr);
    ~HelpDialog();

private slots:
    void onTopicChanged(QTreeWidgetItem *item, QTreeWidgetItem *previous);
    void onSearchChanged(const QString &text);

private:
    void createContent();
    void populateTopics();
    QString getTutorialContent();
    QString getQuestionContent();
    QString getConsultContent();
    QString getAboutContent();

    // UI组件
    QSplitter *m_splitter;
    QTreeWidget *m_topicTree;
    QTextBrowser *m_contentBrowser;
    QLineEdit *m_searchEdit;
    QPushButton *m_closeButton;

    // 帮助内容数据
    QMap<QString, QString> m_helpContent;

    // 默认显示主题
    QString m_defaultTopic;
};

#endif // HELPDIALOG_H
