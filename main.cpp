#include <QCoreApplication>
#include <QtNetwork>
#include "mainserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainServer mainServer;

    mainServer.listen();

    return a.exec();
}
