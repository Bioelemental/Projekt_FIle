#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "llmclient.h"
#include "scriptrunner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Główne okno aplikacji FileOrganizer
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseButton_clicked();
    void on_previewButton_clicked();
    void on_runButton_clicked();
    void onScriptReady(const QString &script);
    void onScriptFinished(const QString &output);
    void onError(const QString &error);

private:
    Ui::MainWindow *ui;
    LlmClient *llmClient;
    ScriptRunner *scriptRunner;
    QString currentScript;
    void setButtonsEnabled(bool enabled);
};

#endif // MAINWINDOW_H