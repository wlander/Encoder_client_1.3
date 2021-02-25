#include "myudp.h"
#include <QMessageBox>
#include <QString>

MyUDP::MyUDP(QObject *parent) : 
    QObject(parent)
{
  socket = new QUdpSocket(this);
  
  //We need to bind the UDP socket to an address and a port
  socket->bind(QHostAddress::AnyIPv4, 45454);         //ex. Address localhost, port 1234

  connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));

   //qDebug("Hello MyUDP!");
   qDebug()<<"Hello MyUDP! "<<" LocalHost: "<<QString::number(QHostAddress::LocalHost);

}

MyUDP::~MyUDP()
{
    qDebug("By in Thread_UDP!");
}

void MyUDP::SayHello()      //Just spit out some data
{
  QByteArray Data;
  Data.append("Hello from UDP land");
  
  socket->writeDatagram(Data,QHostAddress::LocalHost, 45454);
  
  //If you want to broadcast something you send it to your broadcast address
  //ex. 192.2.1.255
}


void MyUDP::readyRead()     //Read something
{
  QByteArray Buffer;
  QString str;
  Buffer.resize(socket->pendingDatagramSize());
  //Buffer.resize(12);
  QHostAddress sender = QHostAddress::AnyIPv4;
  quint16 senderPort = 45454;
  socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);
  
//  str = QHostAddress::LocalHost;
//  str = QString::number(QHostAddress("192.255.255.255"));

  qDebug("data recieve!");
  qDebug()<<"data: "<<Buffer.data();

  //The address will be sender.toString()
}
