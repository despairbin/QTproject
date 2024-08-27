#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QTcpServer *myTcpServer;
    QTcpSocket *myTcpSocket;

    QTcpSocket *clientTcpSocket;
    QHostAddress  client_ip;
    quint16       client_port;

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
