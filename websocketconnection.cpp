#include "websocketconnection.h"

WebsocketConnection::WebsocketConnection(
    QTcpSocket *tcpSocket,
    QObject *parent
) :
    QObject(parent),
    tcpClient(tcpSocket),
    isUpgraded(false)
{
    tcpClient->setParent(this);

    connect(tcpClient, &QTcpSocket::readyRead,
            this, &WebsocketConnection::handleRead);
}

void WebsocketConnection::handleRead()
{
    if (isUpgraded) {
        handleWebsocketMessage();
    } else {
        handleHttpUpgrade();
    }
}

void WebsocketConnection::handleWebsocketMessage()
{
    const QByteArray messageBytes = tcpClient->readAll();
    unsigned char opcode = messageBytes.at(0) & 0b00001111;
    bool isMasked = messageBytes.at(1) & 0b10000000;
    quint64 payloadLength;

    unsigned char shortPayloadLength = messageBytes.at(1) & 0b01111111;
    if (shortPayloadLength == 126) {
        payloadLength = messageBytes.sliced(2, 2).toUInt();
    } else if (shortPayloadLength == 127) {
        payloadLength = messageBytes.sliced(2, 8).toULongLong();
    } else {
        payloadLength = shortPayloadLength;
    }

    qDebug() << "opcode " << opcode;
    qDebug() << "isMasked " << isMasked;
    qDebug() << "payload length " << payloadLength;

    QString payloadData;
    if (isMasked) {

    }
}

void WebsocketConnection::handleHttpUpgrade()
{
    QByteArray messageBytes = tcpClient->readAll();
    QString messageStr(messageBytes);

    qDebug() << messageStr;

    QStringList messageLines = messageStr.split("\r\n", Qt::SkipEmptyParts);
    messageLines.removeFirst();

    QMap<QString, QString> headers;
    QStringList::const_iterator messageLinePtr;
    for (
         messageLinePtr = messageLines.constBegin();
         messageLinePtr != messageLines.constEnd();
         ++messageLinePtr
    ) {
        QString messageLine = *messageLinePtr;
        headers.insert(messageLine.section(": ", 0, 0), messageLine.section(": ", 1));
    }

    qDebug() << headers;

    QString secWebsocketAcceptUnhashed;
    secWebsocketAcceptUnhashed.append(headers["Sec-WebSocket-Key"]);
    secWebsocketAcceptUnhashed.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    QString secWebSocketAccept = QCryptographicHash::hash(
        secWebsocketAcceptUnhashed.toUtf8(),
        QCryptographicHash::Sha1
    ).toBase64();

    QString response;

    response.append("HTTP/1.1 101 Switching Protocols\r\n");
    response.append("Upgrade: websocket\r\n");
    response.append("Connection: Upgrade\r\n");
    response.append("Sec-WebSocket-Accept: ");
    response.append(secWebSocketAccept);
    response.append("\r\n");
    response.append("\r\n");

    tcpClient->write(response.toUtf8());

    isUpgraded = true;
}
