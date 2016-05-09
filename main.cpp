#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QIcon icon(":/icons/search16.png");

    w.setWindowTitle("QtSlovopedia");
    w.setWindowIcon(icon);
    w.show();

    return a.exec();
}
