#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "fileprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseInputButton_clicked();
    void on_browseOutputButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void updateProgress(int value);
    void processingFinished();
    void fileProcessed(QString filename);
    void on_timerCheckBox_stateChanged(int state);

private:
    Ui::MainWindow *ui;
    FileProcessor *processor;
    QTimer *timer;
    void updateTimerState();
    void saveSettings();
    void loadSettings();
};
#endif // MAINWINDOW_H