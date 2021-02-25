#include "port.h"
#include <qdebug.h>

extern volatile int N_RCV;

Port::Port(QObject *parent) :
    QObject(parent)
{
}

Port::~Port()
{
    qDebug("By in Thread!");
    emit finished_Port();
}

void Port :: process_Port(){
    qDebug("Hello World in Thread!");
    connect(&thisPort,SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(&thisPort, SIGNAL(readyRead()),this,SLOT(ReadInPort()));
}

void Port :: Write_Settings_Port(QString name, int baudrate,int DataBits,
                         int Parity,int StopBits, int FlowControl){
    SettingsPort.name = name;
    SettingsPort.baudRate = (QSerialPort::BaudRate) baudrate;
    SettingsPort.dataBits = (QSerialPort::DataBits) DataBits;
    SettingsPort.parity = (QSerialPort::Parity) Parity;
    SettingsPort.stopBits = (QSerialPort::StopBits) StopBits;
    SettingsPort.flowControl = (QSerialPort::FlowControl) FlowControl;

}

void Port :: ConnectPort(void){//
    thisPort.setPortName(SettingsPort.name);
    if (thisPort.open(QIODevice::ReadWrite)) {
        if (thisPort.setBaudRate(SettingsPort.baudRate)
                && thisPort.setDataBits(SettingsPort.dataBits)//DataBits
                && thisPort.setParity(SettingsPort.parity)
                && thisPort.setStopBits(SettingsPort.stopBits)
                && thisPort.setFlowControl(SettingsPort.flowControl))
        {
            if (thisPort.isOpen()){
                error_((SettingsPort.name+ " >> Открыт!\r").toLocal8Bit());
            }
        } else {
            thisPort.close();
            error_(thisPort.errorString().toLocal8Bit());
          }
        thisPort.setReadBufferSize(nrcv*16); //!!! *2
    } else {
        thisPort.close();
        error_(thisPort.errorString().toLocal8Bit());
    }
}

void Port::handleError(QSerialPort::SerialPortError error)//
{
    if ( (thisPort.isOpen()) && (error == QSerialPort::ResourceError)) {
        error_(thisPort.errorString().toLocal8Bit());
        DisconnectPort();
    }
}//


void  Port::DisconnectPort(){
    if(thisPort.isOpen()){
        thisPort.close();
        error_(SettingsPort.name.toLocal8Bit() + " >> Закрыт!\r");
    }
}
//ot tuta kovuratji!!!
void Port :: WriteToPort(QByteArray data){
    if(thisPort.isOpen()){
        thisPort.write(data);
    }
}
//
void Port :: ReadInPort(){
    QByteArray data;
    int Cnt_Byte = 0;

//    data = thisPort.readAll(); //!!! tak bylo

    if(num_buf==false){
        buf1.append(thisPort.readAll());
        Cnt_Byte = buf1.count();
    }
    else{
        buf2.append(thisPort.readAll());
        Cnt_Byte = buf2.count();
    }


#ifdef DEBUG_MODE
    qDebug()<<"принято байт: "<<data.size();
#endif

//    outPort(data);
#ifndef HEADER_TYPE2
    if(Cnt_Byte>=nrcv*2+2){
#else
    if(Cnt_Byte>=nrcv*2+4){
#endif
        Cnt_Byte = 0;

        if(num_buf==false){
           buf2.clear();
           num_buf = true;
           outPort(buf1);
        }
        else{
            buf1.clear();
            num_buf = false;
            outPort(buf2);
        }

    }

}

void Port :: SetRunRecv(bool Recv_En){
   RecvEn = Recv_En;
}
void Port :: SetRunReg(bool Reg_En){
   RegEn = Reg_En;
}

