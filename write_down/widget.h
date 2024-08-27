#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QLabel>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void on_btn_start_clicked();

    void on_btn_clear_clicked();

    void on_btn_config_clicked();

    void onTimerTimeout();

private:
    Ui::Widget *ui;
    bool painting;
    QImage image;
    QPoint lastPoint;
    bool writingEnabled;
    int cnt;
    QTimer *timer;
    bool powerSavingMode;
    int idleTime;

    void saveImage();
};

#endif // WIDGET_H
