#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDataStream>
#include <QtSql>
#include <QAction>
#include <QMap>
#include <QtGlobal>
#include <QSqlQueryModel>
#include <QDataWidgetMapper>
#include <QMessageBox>

#include "scanthread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void scanVocabulary(bool toggled);
    void stopScan(bool toggled);
    void updateButtons(int row);
    void submitButton(void);
    void selectVocabulary(int index);
    void scanFinished(void);
    void updateStatusBar(QString str);
    void setupModel(void);
    void exportVocabulary(void);

protected:
    void changeEvent(QEvent *e);
    bool event(QEvent *e);

private:
    QSqlDatabase db;
    Ui::MainWindow *ui;

    QFont font;
    QAction *actionExit, *actionScan;
    ScanThread *thread;

    QSqlQueryModel *model;
    QDataWidgetMapper *mapper;

    QMap<int, QString> vocabulary;

    int vocabularyId;
};

#endif // MAINWINDOW_H
