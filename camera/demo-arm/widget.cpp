#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    width=320;
    hight=240;
    cam_raw_buf_len=width * width * 4;
    cam_rgb_buf_len=width*width*4;
    cam_raw_buf=(unsigned char *)malloc(cam_raw_buf_len);
    timer=new QTimer(this);
    image=new QImage(width,hight,QImage::Format_RGB888);
    pix =new QPixmap(width,hight);
    timer->setInterval(1000/24);
    if(cam_rgb_buf==(void *)-1)
    {
        QMessageBox::critical(this,"ERROR","malloc  Error!!");
        this->close();
    }
    cam_rgb_buf=(unsigned char *)malloc(cam_rgb_buf_len);

    if(cam_raw_buf==(void *)-1)
    {
        QMessageBox::critical(this,"ERROR","malloc  Error!!");
        this->close();
    }

    cam_vd=new VideoDevice("/dev/video0");

    if(-1==cam_vd->open_device())
    {
        QMessageBox::critical(this,"ERROR","Open Device Error!!");
        this->close();
    }

    cam_vd->init_device();
    qDebug()<<"1";
    cam_vd->start_capturing();
    qDebug()<<"2";


    connect(timer,&QTimer::timeout,this,[=](){

        cam_vd->get_frame((void **)&cam_raw_buf,(size_t *)&cam_raw_buf_len);

        yuyv422_to_rgb888(cam_raw_buf,cam_rgb_buf,width,hight);
        curDateTime=QDateTime::currentDateTime();

        *image=QImage(cam_rgb_buf,width,hight,QImage::Format_RGB888);
        *pix=QPixmap::fromImage(*image).scaled(ui->label->size());

        static QTime time(QTime::currentTime());//
        double key = time.elapsed()/1000.0;
        this->repaint();
        static double lastFpsKey = 0;
        static int  frameCount;
        pen.setColor(Qt::darkGreen);


        QFont font;
        font.setPointSize(20);//设置字体大小

        QPainter painter(pix);
        painter.setFont(font);

        painter.setPen(pen);
        ++frameCount;
        painter.drawText(10,30,QString("FPS: %1").arg(frameCount/(key-lastFpsKey)));
        painter.drawText(10,60,"吴潮斌");
        painter.drawText(10,240,curDateTime.toString("2022-07-06 HH:mm:ss"));
        painter.end();
        if(key - lastFpsKey>1){
            qDebug()<<QString("%1 FPS").arg(frameCount/(key-lastFpsKey), 0, 'f', 0);

            lastFpsKey = key;
            frameCount = 0;

        }



        ui->label->setPixmap(*pix);
        cam_vd->unget_frame();

    });

    timer->start();




}

Widget::~Widget()
{
    delete ui;
}

void Widget::yuyv422_to_rgb888(unsigned char *yuyvdata, unsigned char
                               *rgbdata, int w, int h)
{
    //码流Y0 U0 Y1 V1 Y2 U2 Y3 V3 --》YUYV像素[Y0 U0 V1] [Y1 U0 V1] [Y2 U2 V3]
    //[Y3 U2 V3]--》RGB像素
    int r1, g1, b1;
    int r2, g2, b2;
    int i;
    for(i=0; i<w*h/2; i++)
    {
        char data[4];
        memcpy(data, yuyvdata+i*4, 4);
        unsigned char Y0=data[0];
        unsigned char U0=data[1];
        unsigned char Y1=data[2];
        unsigned char V1=data[3];
        r1 = Y0+1.4075*(V1-128); if(r1>255)r1=255; if(r1<0)r1=0;
        g1 =Y0- 0.3455 * (U0-128) - 0.7169*(V1-128); if(g1>255)g1=255;
        if(g1<0)g1=0;
        b1 = Y0 + 1.779 * (U0-128); if(b1>255)b1=255; if(b1<0)b1=0;
        r2 = Y1+1.4075*(V1-128);if(r2>255)r2=255; if(r2<0)r2=0;
        g2 = Y1- 0.3455 * (U0-128) - 0.7169*(V1-128); if(g2>255)g2=255;
        if(g2<0)g2=0;
        b2 = Y1 + 1.779 * (U0-128); if(b2>255)b2=255; if(b2<0)b2=0;
        rgbdata[i*6+0]=r1;
        rgbdata[i*6+1]=g1;
        rgbdata[i*6+2]=b1;
        rgbdata[i*6+3]=r2;
        rgbdata[i*6+4]=g2;
        rgbdata[i*6+5]=b2;
    }
}
