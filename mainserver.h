#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QObject>
#include <QtNetwork>

class MainServer : public QObject
{
    Q_OBJECT

public:
    explicit MainServer(QObject *parent = nullptr);
    void listen();

private:
    QTcpServer *tcpServer;

private slots:
    void handleNewConnection();

signals:

};

#endif // MAINSERVER_H
