#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->textRecv->setReadOnly(true);
    this->udpSocketSRV = new QUdpSocket(this);
    peer_ip = QHostAddress("0.0.0.0");
    peer_port = 0;
    udpSocketSRV->bind(QHostAddress::Any,8888);

    connect(udpSocketSRV,&QUdpSocket::readyRead,this,[=](){
        char rcvbuf[2048] = {0};

        qint64 rcv_len = 0;
        rcv_len = udpSocketSRV->readDatagram(rcvbuf,udpSocketSRV->pendingDatagramSize(),&peer_ip,&peer_port);
        if(rcv_len > 0){
            QTextCodec *gbk = QTextCodec::codecForName("GBK");
            QString string = gbk->toUnicode(rcvbuf);
            QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
            QString string1 = utf8->fromUnicode(string);

            ui->textRecv->append(QString("%1:%2->%3").arg(peer_ip.toString()).arg(peer_port).arg(string1.data()));
        }
    });


}

Widget::~Widget()
{
    delete ui;
}



void Widget::on_but_send_clicked()
{
    QString sendMsg = ui->lineSend->toPlainText();
    peer_ip = QHostAddress("192.168.43.61");
    peer_port = 9999;
    if(!(sendMsg.isEmpty())){
        sendMsg.append("\r\n");
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
        QString string = utf8->toUnicode(sendMsg.toUtf8().data());
        QTextCodec *gbk = QTextCodec::codecForName("GBK");
        QByteArray array = gbk->fromUnicode(string);


        udpSocketSRV->writeDatagram(array,sendMsg.size(),peer_ip,peer_port);
     }
    qDebug() <<"Remote IP address is:" << peer_ip <<"Remote port:" <<peer_port;
}

void Widget::on_but_quit_clicked()
{
    this->close();
}
