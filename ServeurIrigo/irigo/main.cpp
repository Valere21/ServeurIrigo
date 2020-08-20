#include <QCoreApplication>
#include <QApplication>
#include "mainwindow.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //Serveur b;

    w.show();


    return a.exec();
}
