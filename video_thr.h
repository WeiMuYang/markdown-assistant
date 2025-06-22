#ifndef VIDEOTHR_H
#define VIDEOTHR_H

#include <QThread>
#include <QObject>
#include <QImage>

class VideoThr : public QThread
{
    Q_OBJECT
public:
    VideoThr();
    void setVideoPath(const QString &videoPath)
    {
        this->videoPath_ = videoPath;
    }

    void play();

    void stop(){
        stopIsTrue_ = true;
    }

    bool getFirstVideoFrame(const QString &videoPath,QIcon &ico);

signals:
    void sigCreateVideoFrame(QImage image);
    void sigLogBoxMsg(QString, int ,QString);
    void sigVideoThrLogText(QString log);

protected:
    void run();

private:
    QString videoPath_;
    QImage* videoFirstImage_;
    bool stopIsTrue_{false};
};

#endif // VIDEOTHR_H
