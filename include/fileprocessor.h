#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QVector>

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);
    void setSettings(const QString &inputMask, const QString &outputPath, 
                    bool deleteInput, bool overwrite, const QVector<quint8> &xorKey);
    void processFiles();
    void stopProcessing();

signals:
    void finished();
    void fileProcessed(QString filename);
    void errorOccurred(QString message);

private:
    QString inputMask;
    QString outputPath;
    bool deleteInput;
    bool overwrite;
    QVector<quint8> xorKey;
    bool stopped;
    
    QString getUniqueFilename(const QString &path) const;
    bool processFile(const QString &inputPath, const QString &outputPath);
};

#endif // FILEPROCESSOR_H
