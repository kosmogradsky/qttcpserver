#include <QCoreApplication>
#include <QtNetwork>

//#include "mainserver.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}

static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    MainServer mainServer;

//    mainServer.listen();

    const AVInputFormat *avInputFormat = NULL;
    while (true) {
        avInputFormat = av_input_video_device_next(avInputFormat);

        AVFormatContext *avFormatContext = avformat_alloc_context();

        int openInputError = avformat_open_input(&avFormatContext, "video=\"Integrated Camera\"", avInputFormat, NULL);
        const char *openInputErrorStr = av_make_error(openInputError);

        qDebug() << "hello";
    }

//    AVDeviceInfo *avDeviceInfo = avDeviceInfoList->devices;

    return a.exec();
}
