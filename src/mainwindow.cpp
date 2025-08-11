#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    processor = new FileProcessor(this);
    timer = new QTimer(this);

    connect(processor, &FileProcessor::finished, this, &MainWindow::processingFinished);
    connect(processor, &FileProcessor::fileProcessed, this, &MainWindow::fileProcessed);
    connect(timer, &QTimer::timeout, this, [this]() { processor->processFiles(); });
    
    loadSettings();
    updateTimerState();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::on_browseInputButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Input Directory");
    if (!dir.isEmpty()) {
        ui->inputDirEdit->setText(dir);
    }
}

void MainWindow::on_browseOutputButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (!dir.isEmpty()) {
        ui->outputDirEdit->setText(dir);
    }
}

void MainWindow::on_startButton_clicked()
{
    if (ui->inputDirEdit->text().isEmpty() || ui->outputDirEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify input and output directories");
        return;
    }
    
    QVector<quint8> xorKey;
    const QStringList bytes = ui->xorKeyEdit->text().split(' ', Qt::SkipEmptyParts);
    for (const QString &byte : std::as_const(bytes)) {
        bool ok;
        quint8 value = byte.toUInt(&ok, 16);
        if (ok) {
            xorKey.append(value);
        }
    }
    
    if (xorKey.size() != 8) {
        QMessageBox::warning(this, "Error", "XOR key must be exactly 8 bytes (16 hex characters)");
        return;
    }
    
    processor->setSettings(
        ui->inputDirEdit->text() + "/" + ui->fileMaskEdit->text(),
        ui->outputDirEdit->text(),
        ui->deleteInputCheckBox->isChecked(),
        ui->overwriteCheckBox->isChecked(),
        xorKey
    );
    
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    
    if (!ui->timerCheckBox->isChecked()) {
        processor->processFiles();
    } else {
        timer->start(ui->intervalSpinBox->value() * 1000);
    }
}

void MainWindow::on_stopButton_clicked()
{
    processor->stopProcessing();
    timer->stop();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}
void MainWindow::processingFinished()
{
    timer->stop();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    QMessageBox::information(this, "Done", "File processing completed");
}

void MainWindow::fileProcessed(QString filename)
{
    ui->logTextEdit->appendPlainText("Processed: " + filename);
}

void MainWindow::on_timerCheckBox_stateChanged(int state)
{
    updateTimerState();
}

void MainWindow::updateTimerState()
{
    bool timerEnabled = ui->timerCheckBox->isChecked();
    ui->intervalSpinBox->setEnabled(timerEnabled);
    ui->intervalLabel->setEnabled(timerEnabled);
}

void MainWindow::saveSettings()
{
    QSettings settings("FileProcessor", "FileProcessor");
    settings.setValue("inputDir", ui->inputDirEdit->text());
    settings.setValue("outputDir", ui->outputDirEdit->text());
    settings.setValue("fileMask", ui->fileMaskEdit->text());
    settings.setValue("deleteInput", ui->deleteInputCheckBox->isChecked());
    settings.setValue("overwrite", ui->overwriteCheckBox->isChecked());
    settings.setValue("timerEnabled", ui->timerCheckBox->isChecked());
    settings.setValue("interval", ui->intervalSpinBox->value());
    settings.setValue("xorKey", ui->xorKeyEdit->text());
}

void MainWindow::loadSettings()
{
    QSettings settings("FileProcessor", "FileProcessor");
    ui->inputDirEdit->setText(settings.value("inputDir").toString());
    ui->outputDirEdit->setText(settings.value("outputDir").toString());
    ui->fileMaskEdit->setText(settings.value("fileMask", "*.*").toString());
    ui->deleteInputCheckBox->setChecked(settings.value("deleteInput", false).toBool());
    ui->overwriteCheckBox->setChecked(settings.value("overwrite", true).toBool());
    ui->timerCheckBox->setChecked(settings.value("timerEnabled", false).toBool());
    ui->intervalSpinBox->setValue(settings.value("interval", 60).toInt());
    ui->xorKeyEdit->setText(settings.value("xorKey", "00 00 00 00 00 00 00 00").toString());
}
