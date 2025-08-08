#include "fileprocessor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QDebug>

FileProcessor::FileProcessor(QObject *parent) : QObject(parent), stopped(false)
{
}

void FileProcessor::setSettings(const QString &inputMask, const QString &outputPath, 
                              bool deleteInput, bool overwrite, const QVector<quint8> &xorKey)
{
    this->inputMask = inputMask;
    this->outputPath = outputPath;
    this->deleteInput = deleteInput;
    this->overwrite = overwrite;
    this->xorKey = xorKey;
    this->stopped = false;
}

void FileProcessor::processFiles()
{
    stopped = false;
    
    QFileInfo const maskInfo(inputMask);
    QDir const inputDir(inputMask);
    QStringList const files = inputDir.entryList({maskInfo.fileName()}, QDir::Files);
    if (files.isEmpty()) {
        emit finished();
        return;
    }
    
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, [this, watcher]() {
        watcher->deleteLater();
        emit finished();
    });
    
    connect(watcher, &QFutureWatcher<void>::progressValueChanged, this, &FileProcessor::progressChanged);
    
    QFuture<void> future = QtConcurrent::map(files, [this](const QString &file) {
        if (stopped) return;
        
        QString inputPath = QFileInfo(inputMask).path() + "/" + file;
        QString outputFile = this->outputPath + "/" + file;
        
        if (!overwrite) {
            outputFile = getUniqueFilename(outputFile);
        }
        
        if (processFile(inputPath, outputFile)) {
            emit fileProcessed(file);
            if (deleteInput) {
                QFile::remove(inputPath);
            }
        }
    });
    
    watcher->setFuture(future);
}

void FileProcessor::stopProcessing()
{
    stopped = true;
}

QString FileProcessor::getUniqueFilename(const QString &path) const
{
    QFileInfo fileInfo(path);
    QString baseName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();
    QString dir = fileInfo.path();
    
    QString newPath = path;
    int counter = 1;
    
    while (QFileInfo::exists(newPath)) {
        newPath = dir + "/" + baseName + "_" + QString::number(counter) + "." + suffix;
        counter++;
    }
    
    return newPath;
}

bool FileProcessor::processFile(const QString &inputPath, const QString &outputPath)
{
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open input file: " + inputPath);
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        inputFile.close();
        emit errorOccurred("Cannot create output file: " + outputPath);
        return false;
    }
    
    const qint64 bufferSize = 1024 * 1024; // 1MB buffer
    QByteArray buffer;
    buffer.resize(bufferSize);
    
    qint64 totalBytes = inputFile.size();
    qint64 bytesProcessed = 0;
    
    while (!inputFile.atEnd() && !stopped) {
        qint64 bytesRead = inputFile.read(buffer.data(), buffer.size());
        if (bytesRead == -1) {
            emit errorOccurred("Error reading file: " + inputPath);
            break;
        }
        
        // Perform XOR operation
        for (qint64 i = 0; i < bytesRead; ++i) {
            buffer[i] = buffer[i] ^ xorKey[i % 8];
        }
        
        if (outputFile.write(buffer.constData(), bytesRead) == -1) {
            emit errorOccurred("Error writing to file: " + outputPath);
            break;
        }
        
        bytesProcessed += bytesRead;
        int progress = static_cast<int>((bytesProcessed * 100) / totalBytes);
        emit progressChanged(progress);
    }
    
    inputFile.close();
    outputFile.close();
    
    return !stopped && (bytesProcessed == totalBytes);
}
