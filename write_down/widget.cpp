#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QPixmap>
#include <QFileDialog>
#include <cstdio>
#include <QTimer>

//#include <QBuffer>
//#include <QIODevice>

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    writingEnabled = false;
    ui->btn_clear->setEnabled(false);
    ui->btn_config->setEnabled(false);
    cnt = 1;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    timer->setInterval(1000); // 设置定时器间隔为1分钟，单位为毫秒


    setFixedSize(480, 272);
    painting = false;
    image = QImage(480, 272, QImage::Format_RGB32);
    image.fill(Qt::white);
    timer->start();
    powerSavingMode = false;
    idleTime = 0;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
            painting = true;
            lastPoint = event->pos();
    if (powerSavingMode && !writingEnabled) {
        powerSavingMode = false;
        repaint();
        timer->start();
        }
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton)) {
        if(writingEnabled == true){
        // 获取 Label 的相对位置
        QPoint labelPos = ui->area_writedown->mapTo(this, QPoint(0, 0));

        // 获取 Label 的尺寸
        QSize labelSize = ui->area_writedown->size();

        // 创建 Label 区域
        QRect labelRect(labelPos, labelSize);

        // 检查鼠标位置是否在 Label 区域内
        if (labelRect.contains(event->pos())) {
            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(lastPoint, event->pos());
            lastPoint = event->pos();
            update();
            }
        }
        powerSavingMode = false;  // 用户操作，退出省电模式
        timer->start();
    }

}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
            if(painting == true) painting = false;
            powerSavingMode = false;  // 用户操作，退出省电模式
            timer->start();
        }

}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(0, 0, image);
    // 如果进入省电模式，绘制黑色背景
    if (powerSavingMode && !writingEnabled)
        painter.fillRect(rect(), Qt::black);
    else if(!powerSavingMode && !writingEnabled)
        painter.fillRect(rect(), Qt::white);
}
void Widget::on_btn_clear_clicked()
{
    // 清空绘制
    image.fill(Qt::white);
    powerSavingMode = false;  // 退出省电模式
    repaint();  // 重新绘制界面
}

void Widget::on_btn_start_clicked()
{
    // 启用绘图
    writingEnabled = true;
    ui->btn_clear->setEnabled(true);
    ui->btn_config->setEnabled(true);
    idleTime = 0;  // 重置空闲时间
    image.fill(Qt::white);
    powerSavingMode = false;  // 退出省电模式
    timer->start();
    repaint();  // 重新绘制界面
}

void Widget::onTimerTimeout()
{
    if (!writingEnabled) {
        idleTime += timer->interval();  // 累计空闲时间
        if (idleTime >= 1000) {  // 如果空闲时间达到1秒钟
            powerSavingMode = true;  // 进入省电模式（屏幕变黑）
            repaint();  // 重新绘制界面
        }
        else powerSavingMode = false;
    } else {
        // 如果进入编辑模式，重置空闲时间
        idleTime = 0;
    }
}

void Widget::on_btn_config_clicked()
{
    // 保存图片
    /*QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg)");
    if (fileName.isEmpty()) {
        return;
    }
    image.save(fileName);
    */
    QString fileName = "drawing.png";

    fileName = QString("drawing%1.png").arg(cnt);

    if(image.save(fileName))
    qDebug() << "Image saved as" << fileName;
    else
    qDebug() << "Save Error!!" ;

    if(cnt > 99)
    {
        qDebug() << "Memory Limit Exceeded";
        qDebug() << "The file will be overwritten from the beginning";
        cnt = 1;
    } else cnt++;

    image.fill(Qt::white);
    update();

    writingEnabled = false;
    ui->btn_clear->setEnabled(false);
    ui->btn_config->setEnabled(false);
    powerSavingMode = false;  // 用户操作，退出省电模式
    timer->start();

}
