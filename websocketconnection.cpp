#include "websocketconnection.h"

static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

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

    const AVCodec *avCodec = avcodec_find_decoder(AVCodecID::AV_CODEC_ID_H264);
    avCodecCtx = avcodec_alloc_context3(avCodec);

    avCodecCtx->width = 640;
    avCodecCtx->height = 480;
    avCodecCtx->bit_rate = 2000000;

    AVRational framerate;
    framerate.num = 30;
    framerate.den = 1;
    avCodecCtx->framerate = framerate;

    avcodec_open2(avCodecCtx, avCodec, NULL);
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

    unsigned char charPayloadLength = messageBytes.at(1) & 0b01111111;
    unsigned char maskingKeyStart = 2;
    if (charPayloadLength == 126) {
        QDataStream lengthStream(messageBytes.sliced(2, 2));
        unsigned short shortPayloadLength;

        lengthStream >> shortPayloadLength;
        payloadLength = shortPayloadLength;
        maskingKeyStart += 2;
    } else if (charPayloadLength == 127) {
        payloadLength = messageBytes.sliced(2, 8).toULongLong();
        maskingKeyStart += 8;
    } else {
        payloadLength = charPayloadLength;
    }

    qDebug() << "byte array length " << messageBytes.size();
    qDebug() << "opcode " << opcode;
    qDebug() << "isMasked " << isMasked;
    qDebug() << "payload length " << payloadLength;

    QString payloadData;
    if (isMasked) {
        const QByteArray maskingKey = messageBytes.sliced(maskingKeyStart, 4);
        const QByteArray maskedPayload = messageBytes.sliced(maskingKeyStart + 4, payloadLength);
        char j;
        QByteArray unmaskedPayload;

        qDebug() << "masked payload " << maskedPayload;

        for (qsizetype i = 0; i < maskedPayload.size(); ++i) {
            j = i % 4;
            unmaskedPayload.append(maskedPayload.at(i) ^ maskingKey.at(j));
        }

        payloadData = QString(unmaskedPayload);

        qDebug() << "unmasked payload " << unmaskedPayload;
        qDebug() << "payload data " << payloadData;

        QDataStream unmaskedPayloadStream(unmaskedPayload);
        int packetSize = unmaskedPayload.size();
        char *packetRawData = (char *) av_malloc(packetSize);
        char *incPacketRawData = packetRawData + 5;

        unmaskedPayloadStream.readRawData(packetRawData, packetSize);

        AVPacket *avPacket = av_packet_alloc();
        AVFrame *avFrame = av_frame_alloc();

        av_packet_from_data(avPacket, (uint8_t*) packetRawData, packetSize);
        char sendPacketResponse = avcodec_send_packet(avCodecCtx, avPacket);
        const char *sendPacketError = av_make_error(sendPacketResponse);
        char receiveFrameResponse = avcodec_receive_frame(avCodecCtx, avFrame);
        const char *receiveFrameError = av_make_error(receiveFrameResponse);

        qDebug() << "hello";
    } else {
        // error
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
