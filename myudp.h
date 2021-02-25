//Base class QObject

#include <QUdpSocket>
 
class MyUDP : public QObject
{
  Q_OBJECT
 
  public:
      explicit MyUDP(QObject *parent = 0);
      ~MyUDP();
      void SayHello();

      QUdpSocket *socket;
 
  signals:
 
  public slots:
      void readyRead();
 
};
