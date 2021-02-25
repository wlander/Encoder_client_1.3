#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //MyUDP server;
//    MyServer server;

    //server.SayHello();
    return a.exec();
}
