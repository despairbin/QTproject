#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    myTcpServer  =  new QTcpServer(this);
    myTcpSocket = new QTcpSocket(this);

    clientTcpSocket = new QTcpSocket(this);

    client_ip.setAddress("127.0.0.1");
    client_port = 0;

    ui->leIpAddress->setText("192.168.211.1");


    ui->connectBTN->setEnabled(true);
    ui->disConnectBTN->setEnabled(false);


    myTcpServer->listen(QHostAddress::Any,8888);

    //connect(myTcpServer,SIGNAL(newConnection()),this,SLOT(myfunc()));
    connect(myTcpServer,&QTcpServer::newConnection,this,[=](){
        myTcpSocket = myTcpServer->nextPendingConnection();
        QString msg = QString("new incoming %1:%2->").arg(myTcpSocket->peerAddress().toString()).arg(myTcpSocket->peerPort());
        ui->peRcvMSG->appendPlainText(msg);


        connect(myTcpSocket,&QTcpSocket::readyRead,this,[=](){
            QString rcvbuf = QString("%1:%2->").arg(myTcpSocket->peerAddress().toString()).arg(myTcpSocket->peerPort());
            rcvbuf.append(myTcpSocket->readAll());


            ui->peRcvMSG->appendPlainText(rcvbuf);

        });

    });


    connect(ui->connectBTN,&QPushButton::clicked,this,[=](){

        client_ip = QHostAddress(ui->leIpAddress->text().trimmed());
        client_port =ui->lePort->text().toShort();
        clientTcpSocket->connectToHost(client_ip,client_port);
        qDebug() << clientTcpSocket->state();


        connect(clientTcpSocket,&QTcpSocket::connected,this,[=](){
            ui->connectBTN->setDisabled(true);
            ui->disConnectBTN->setEnabled(true);
            connect(ui->sendMsgBTN,&QPushButton::clicked,this,[=](){
                clientTcpSocket->write(ui->leSendMSG->text().toUtf8());

                ui->leSendMSG->clear();
            });


            connect(ui->disConnectBTN,&QPushButton::clicked,this,[=](){

                   clientTcpSocket->disconnectFromHost();
                    clientTcpSocket->close();
                    ui->connectBTN->setDisabled(false);
                    ui->disConnectBTN->setEnabled(false);



            });
        });



    });








}

Widget::~Widget()
{
    delete ui;
}

