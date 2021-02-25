#include "mytcp.h"

MyServer::MyServer(QObject *parent) :
    QObject(parent),
    firstSocket(NULL)
{
    server = new QTcpServer(this);
    qDebug() << "server listen = " << server->listen(QHostAddress::LocalHost, 2323); //server->listen(QHostAddress::Any, 40);
    connect(server, SIGNAL(newConnection()), this, SLOT(incommingConnection())); // подключаем сигнал "новое подключение" к нашему обработчику подключений
    qDebug() << "LocalHost is: "<<QHostAddress::LocalHost;
}

void MyServer::incommingConnection() // обработчик подключений
{
    QTcpSocket * socket = server->nextPendingConnection(); // получаем сокет нового входящего подключения
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState))); // делаем обработчик изменения статуса сокета
    if (!firstSocket) { // если у нас нет "вещающего", то данное подключение становится вещающим
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead())); // подключаем входящие сообщения от вещающего на наш обработчик
        socket->write("server"); // говорим ему что он "вещает"
        firstSocket = socket; // сохраняем себе"
        qDebug() << "this one is server";
        emit sig_Connect_Socket();
    }
    else { // иначе говорим подключенному что он "получатель"
        socket->write("client");
        sockets << socket;
    }
}

void MyServer::readyRead() // обработчик входящих сообщений от "вещающего"
{
    QObject * object = QObject::sender(); // далее и ниже до цикла идет преобразования "отправителя сигнала" в сокет, дабы извлечь данные
    if (!object)
        return;

    QTcpSocket * socket = static_cast<QTcpSocket *>(object);

    if(socket->bytesAvailable()>=2000*4){

        QByteArray arr =  socket->readAll();

        // на самом деле весь верхний код можно было заменить на firstSocket, но я выдирал код из другого проекта, и переписывать мне лень :)

        foreach(QTcpSocket *socket, sockets) { // пишем входящие данные от "вещающего" получателям
            if (socket->state() == QTcpSocket::ConnectedState)
                socket->write(arr);
        }

       emit sig_readyRead(arr);
   }

   //qDebug() << arr.count();

}
void MyServer::stateChanged(QAbstractSocket::SocketState state) // обработчик статуса, нужен для контроля за "вещающим"
{
    QObject * object = QObject::sender();
    if (!object)
        return;
    QTcpSocket * socket = static_cast<QTcpSocket *>(object);
    //qDebug() << state;
    if (socket == firstSocket && state == QAbstractSocket::UnconnectedState){
        firstSocket = NULL;
        emit sig_DisConnect_Socket();
    }
}
