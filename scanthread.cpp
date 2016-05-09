#include "scanthread.h"

ScanThread::ScanThread(QObject *parent) :
    QThread(parent)
{
    vocabulary = 0;
    startScanning();

    rx.setPattern("\\s\\s+");
    rx1.setPattern("<br>");

    str1 = "<strong>";
    str2 = "</strong>";
    sl1 = str1.length();
    sl2 = str2.length();

    startBlock = "<!-- google_ad_section_start -->";
    endBlock = "<!-- google_ad_section_end -->";
}

ScanThread::~ScanThread()
{
    // скидання списків даних в БД при закриванні потоку
    writeToDB();
}

void ScanThread::run()
{
    QDir dir(path);

    scanDir(dir);
}

void ScanThread::scanDir(QDir &dir)
{
    if (stop)
        return;

    QFileInfoList infoList = dir.entryInfoList();
    QFileInfo fi;
    QString fn;

    int numFiles = infoList.size();
    for (int i = 0; i < numFiles; ++i)
    {
        fi = infoList.at(i);
        fn = fi.fileName();

        if (fn != "." && fn != "..")
        {
            if ( fi.isDir() )
            {
                dir.cd(fn);
                scanDir(dir);
                dir.cdUp();
            }
            else
            {
                parseFile(fi);
                emit parsedFile(fi.filePath());
            }
        }
    }

    writeToDB();
}

void ScanThread::parseFile(QFileInfo &fi)
{
    QString line, out;
    bool bf = false, done = false, ok;
    int idx1 = -1, idx2 = -1;

    file.setFileName(fi.filePath());

    ok = file.open(QIODevice::ReadOnly | QIODevice::Text);

    if ( ok )
    {
        // file opened successfully
        // use a text stream
        ts.setDevice(&file);
        ts.setCodec("UTF-8");

        // until end of file...
        while ( !ts.atEnd() && !done )
        {
            // read and parse the command line
            // line of text excluding '\n'
            line = ts.readLine();

            // do something with the line
            if ( line.contains(startBlock, Qt::CaseInsensitive) )
            {
                bf = true;

                while ( !ts.atEnd() && bf == true )
                {
                    line = ts.readLine();

                    if ( line.contains(endBlock, Qt::CaseInsensitive) )
                    {
                        bf = false;
                        done = true;
                    }
                    else
                    {
                        line = line.remove(rx);
                        out += line;

                        idx1 = line.indexOf(str1);
                        idx2 = line.indexOf(str2);

                        if (idx1 >= 0 && idx2 >= 0)
                        {
                            words << line.mid(idx1+sl1, idx2-(idx1+sl2)+1);
                            idx1 = idx2 = -1;
                        }
                    }
                }
            }
        }

        // If founded startBlock and endBlock, so we have text data
        if ( done )
        {
            text << out;

            out = out.remove(rx1);
            idx2 = out.indexOf(str2);

            if (idx2 >= 0)
            {
                questions << out.mid(idx2+sl2, out.length()-(idx2+sl2)+1);
                idx2 = -1;
            }
        }

        // Close the file
        file.close();
        ts.flush();
    }
    else
        qDebug() << "parseFile: error opening file [" << fi.fileName() << "]";
}

void ScanThread::setDB(QSqlDatabase *sqlDB)
{
    db = sqlDB;
    drv = db->driver();
}

void ScanThread::setPath(QString dir, int key)
{
    path = dir;
    vocabulary = key;
}

void ScanThread::startScanning(void)
{
    stop = false;
}

void ScanThread::stopScanning(void)
{
    stop = true;
}

void ScanThread::writeToDB(void)
{
    if (text.count())
    {
        QString q("INSERT INTO slovopedia.works (_word, _question, _text, _vocabulary) VALUES (?, ?, ?, %1);");
        q = q.arg(vocabulary);

        query.prepare(q);

        drv->beginTransaction();
        query.addBindValue(words);
        query.addBindValue(questions);
        query.addBindValue(text);
        query.execBatch(QSqlQuery::ValuesAsRows);

        le = query.lastError();
        if (le.type() == QSqlError::NoError)
        {
            drv->commitTransaction();

            words.clear();
            questions.clear();
            text.clear();
        }
        else
            qDebug() << "scanDir: [" << le.text() << "]";

        query.finish();
    }
}
