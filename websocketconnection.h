#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include <QObject>
#include <QtNetwork>

class WebsocketConnection : public QObject
{
    Q_OBJECT
public:
    explicit WebsocketConnection(
        QTcpSocket *tcpClient,
        QObject *parent = nullptr
    );

private:
    QTcpSocket *tcpClient;
    bool isUpgraded;

    void handleHttpUpgrade();
    void handleWebsocketMessage();

private slots:
    void handleRead();

signals:

};

#endif // WEBSOCKETCONNECTION_H
