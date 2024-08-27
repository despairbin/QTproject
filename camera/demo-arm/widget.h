#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "videodevice.h"
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QPen>
#include <QPainter>
#include <QTime>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    VideoDevice *cam_vd;

    unsigned char *cam_raw_buf;
    unsigned char *cam_rgb_buf;

    int cam_raw_buf_len;
    int cam_rgb_buf_len;
    int width;
    int hight;
    QImage *image;
    QTimer *timer;
    QPixmap *pix;
    QPen pen;
    QPainter *painter;
    QDateTime curDateTime;
    //QDateTime preDateTime;

    void yuyv422_to_rgb888(unsigned char *yuyvdata,unsigned char *rgbdata,int w,int h);


private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
