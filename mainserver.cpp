#include <QtDebug>
#include "mainserver.h"
#include "websocketconnection.h"

MainServer::MainServer(QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer(this);

    connect(tcpServer, &QTcpServer::newConnection,
            this, &MainServer::handleNewConnection);
}

void MainServer::handleNewConnection() {
    QTcpSocket *tcpClient = tcpServer->nextPendingConnection();
    WebsocketConnection *websocketConnection = new WebsocketConnection(tcpClient);

    connect(tcpClient, &QAbstractSocket::disconnected,
            websocketConnection, &QObject::deleteLater);
}

void MainServer::listen() {
    if (tcpServer->listen(QHostAddress::AnyIPv4, 3000)) {
        qDebug() << "Server started successfully";
    } else {
        qDebug() << "Server could not start!";
    }
}
