#include "video_thr.h"
#include <QString>
#include <QDebug>
#include <QPixmap>
#include <QIcon>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

VideoThr::VideoThr()
{

}

void VideoThr::play()
{
    start();
    stopIsTrue_ = false;
}

bool VideoThr::getFirstVideoFrame(const QString &videoPath,QIcon &ico)
{
    if (videoPath.isNull())
    {
        return false;
    }

    //描述多媒体文件的构成及其基本信息
    AVFormatContext *pAVFormatCtx = avformat_alloc_context();
    pAVFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;
    AVDictionary * opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "2000000", 0);

    if (avformat_open_input(&pAVFormatCtx, videoPath.toLatin1().data(), nullptr, nullptr) != 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"open file fail");
        avformat_free_context(pAVFormatCtx);
        return false;
    }

    //读取一部分视音频数据并且获得一些相关的信息
    if (avformat_find_stream_info(pAVFormatCtx, nullptr) < 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avformat find stream fail");
        avformat_close_input(&pAVFormatCtx);
        return false;
    }

    int iVideoIndex = -1;

    for (uint32_t i = 0; i < pAVFormatCtx->nb_streams; ++i)
    {
        //视频流
        if (pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            iVideoIndex = i;
            break;
        }
    }

    if (iVideoIndex == -1)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"video stream not find");
        avformat_close_input(&pAVFormatCtx);
        return false;
    }

    //获取视频流的编解码器上下文的数据结构
    AVCodecContext *pAVCodecCtx = avcodec_alloc_context3(nullptr);
    if (pAVCodecCtx == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"Could not allocate AVCodecContext \n");
        return  false;
    }
    avcodec_parameters_to_context(pAVCodecCtx, pAVFormatCtx->streams[iVideoIndex]->codecpar);

    if (pAVCodecCtx == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"get codec fail");
        avformat_close_input(&pAVFormatCtx);
        return false;
    }

    //编解码器信息的结构体
    const AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);

    if (pAVCodec == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"not find decoder");
        return false;
    }

    //初始化一个视音频编解码器
    if (avcodec_open2(pAVCodecCtx, pAVCodec, nullptr) < 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avcodec_open2 fail");
        return false;
    }

    //AVFrame 存放从AVPacket中解码出来的原始数据
    AVFrame *pAVFrame = av_frame_alloc();
    AVFrame *pAVFrameRGB = av_frame_alloc();

    //用于视频图像的转换,将源数据转换为RGB32的目标数据
    SwsContext *pSwsCtx = sws_getContext(pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                                         pAVCodecCtx->width, pAVCodecCtx->height, AV_PIX_FMT_RGB32,
                                         SWS_BICUBIC, nullptr, nullptr, nullptr);
    int iNumBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height, 1);

    uint8_t *pRgbBuffer = (uint8_t *)(av_malloc(iNumBytes * sizeof(uint8_t)));
    //为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间
    av_image_fill_arrays(pAVFrameRGB->data, pAVFrameRGB->linesize, pRgbBuffer, AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height, 1);

    //AVpacket 用来存放解码之前的数据
    AVPacket packet;
    av_new_packet(&packet, pAVCodecCtx->width * pAVCodecCtx->height);

    //读取码流中视频帧
    while(av_read_frame(pAVFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index != iVideoIndex)
        {
            //清空packet中data以及buf的内容，并没有把packet本身
            av_packet_unref(&packet);
            continue;
        }

        int iGotPic = -1;
        //解码一帧视频数据
        if(avcodec_send_packet(pAVCodecCtx, &packet)<0 || (iGotPic =avcodec_receive_frame(pAVCodecCtx, pAVFrame))<0)
        {
            av_packet_unref(&packet);
            emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avcodec_decode_video2 fail");
            break;
        }

        if (iGotPic >= 0)
        {
            //转换像素
            sws_scale(pSwsCtx, (uint8_t const * const *)pAVFrame->data, pAVFrame->linesize, 0, pAVCodecCtx->height, pAVFrameRGB->data, pAVFrameRGB->linesize);

            //构造QImage
            videoFirstImage_ = new QImage(pRgbBuffer, pAVCodecCtx->width, pAVCodecCtx->height, QImage::Format_RGB32);
            QPixmap pix = QPixmap::fromImage(*videoFirstImage_);
            ico.addPixmap(pix);
            break; // 只获取一个frame即可
        }
        av_packet_unref(&packet);
        msleep(15);
    }

    //资源回收
    av_free(pAVFrame);
    av_free(pAVFrameRGB);
    sws_freeContext(pSwsCtx);
    avcodec_close(pAVCodecCtx);
    avformat_close_input(&pAVFormatCtx);

    return true;
}

void VideoThr::run()
{
    if (videoPath_.isNull())
    {
        return;
    }
    //描述多媒体文件的构成及其基本信息
    AVFormatContext *pAVFormatCtx = avformat_alloc_context();
    pAVFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;
    AVDictionary * opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "2000000", 0);

    if (avformat_open_input(&pAVFormatCtx, videoPath_.toLatin1().data(), nullptr, nullptr) != 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"open file fail");
        avformat_free_context(pAVFormatCtx);
        return;
    }

    //读取一部分视音频数据并且获得一些相关的信息
    if (avformat_find_stream_info(pAVFormatCtx, nullptr) < 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avformat find stream fail");
        avformat_close_input(&pAVFormatCtx);
        return;
    }

    int iVideoIndex = -1;

    for (uint32_t i = 0; i < pAVFormatCtx->nb_streams; ++i)
    {
        //视频流
        if (pAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            iVideoIndex = i;
            break;
        }
    }

    if (iVideoIndex == -1)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"video stream not find");
        avformat_close_input(&pAVFormatCtx);
        return;
    }

    //获取视频流的编解码器上下文的数据结构
    AVCodecContext *pAVCodecCtx = avcodec_alloc_context3(nullptr);
    if (pAVCodecCtx == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"Could not allocate AVCodecContext \n");
        return  ;
    }
    avcodec_parameters_to_context(pAVCodecCtx, pAVFormatCtx->streams[iVideoIndex]->codecpar);

    if (pAVCodecCtx == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"get codec fail");
        avformat_close_input(&pAVFormatCtx);
        return;
    }

    //编解码器信息的结构体
    const AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);

    if (pAVCodec == nullptr)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"not find decoder");
        return;
    }

    //初始化一个视音频编解码器
    if (avcodec_open2(pAVCodecCtx, pAVCodec, nullptr) < 0)
    {
        emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avcodec_open2 fail");
        return;
    }

    //AVFrame 存放从AVPacket中解码出来的原始数据
    AVFrame *pAVFrame = av_frame_alloc();
    AVFrame *pAVFrameRGB = av_frame_alloc();

    //用于视频图像的转换,将源数据转换为RGB32的目标数据
    SwsContext *pSwsCtx = sws_getContext(pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                                         pAVCodecCtx->width, pAVCodecCtx->height, AV_PIX_FMT_RGB32,
                                         SWS_BICUBIC, nullptr, nullptr, nullptr);
    int iNumBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height, 1);

    uint8_t *pRgbBuffer = (uint8_t *)(av_malloc(iNumBytes * sizeof(uint8_t)));
    //为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间
    av_image_fill_arrays(pAVFrameRGB->data, pAVFrameRGB->linesize, pRgbBuffer, AV_PIX_FMT_RGB32, pAVCodecCtx->width, pAVCodecCtx->height, 1);

    //AVpacket 用来存放解码之前的数据
    AVPacket packet;
    av_new_packet(&packet, pAVCodecCtx->width * pAVCodecCtx->height);

    //读取码流中视频帧
    while (av_read_frame(pAVFormatCtx, &packet) >= 0 && (!stopIsTrue_))
    {
        if (packet.stream_index != iVideoIndex)
        {
            //清空packet中data以及buf的内容，并没有把packet本身
            av_packet_unref(&packet);
            continue;
        }

        int iGotPic = -1;
        //解码一帧视频数据
        if(avcodec_send_packet(pAVCodecCtx, &packet)<0 || (iGotPic =avcodec_receive_frame(pAVCodecCtx, pAVFrame))<0)
        {
            av_packet_unref(&packet);
            emit sigLogBoxMsg(__FUNCTION__, __LINE__,"avcodec_decode_video2 fail");
            break;
        }

        if (iGotPic >= 0)
        {
            //转换像素
            sws_scale(pSwsCtx, (uint8_t const * const *)pAVFrame->data, pAVFrame->linesize, 0, pAVCodecCtx->height, pAVFrameRGB->data, pAVFrameRGB->linesize);

            //构造QImage
            videoFirstImage_ = new QImage(pRgbBuffer, pAVCodecCtx->width, pAVCodecCtx->height, QImage::Format_RGB32);
            //绘制QImage
            emit sigCreateVideoFrame(*videoFirstImage_);
        }
        av_packet_unref(&packet);
        msleep(15);
    }

    //资源回收
    av_free(pAVFrame);
    av_free(pAVFrameRGB);
    sws_freeContext(pSwsCtx);
    avcodec_close(pAVCodecCtx);
    avformat_close_input(&pAVFormatCtx);
}
