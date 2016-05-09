#ifndef SCANTHREAD_H
#define SCANTHREAD_H

#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QVariantList>

#include <QtSql>

class ScanThread : public QThread
{
Q_OBJECT
public:
    explicit ScanThread(QObject *parent = 0);
    ~ScanThread();

    void setDB(QSqlDatabase *sqlDB);
    void setPath(QString dir, int key);

    void startScanning(void);
    void stopScanning(void);

protected:
    void run();
    void scanDir(QDir &dir);
    void parseFile(QFileInfo &fi);

    void writeToDB(void);

signals:
    void parsedFile(QString fn);

public slots:

private:
    int vocabulary, sl1, sl2;
    bool stop;

    QSqlQuery query;
    QSqlError le;
    QSqlDatabase *db;
    QSqlDriver *drv;
    QRegExp rx, rx1;

    QString str1, str2;

    QString path;
    QFile file;
    QTextStream ts;

    QVariantList text;
    QVariantList words;
    QVariantList questions;

    QString startBlock, endBlock;
};

#endif // SCANTHREAD_H
