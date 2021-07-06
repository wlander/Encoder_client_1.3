#include <QDataStream>
#include <qbuffer.h>
#include "port.h"
#include <qdebug.h>
#include <QtCore>


Status::Status(QString lang){
    QMessageBox msgBox;
    if(!(this->set_lang(lang))){
        msgBox.setText("Error language type!");
        msgBox.exec();
    }
    memset(status_num, 0, this->Max_num_Status*sizeof(int));
}

QString Status::get_status(int n){
    return ((lng==0) ? status_list_ru[n]:status_list_eng[n]) + QString::number(status_num[n]);
}

Port::Port(QObject *parent) :
    QObject(parent)
{
    nrcv = 8192;
    nrcv_t = nrcv/sizeof(DATA_TYPE);

    buf_simulator = new QByteArray;
    data_simulator = new float[nrcv];
    timer_simulator = new QTimer;
    q_buffer = new QQueue<QByteArray>;
    data_buf = new QByteArray;

    q_buffer->reserve(nrcv*100*4);
    data_buf->resize(nrcv);
    buf_simulator->resize(nrcv);

    fl_simulator = false;
    cnt_sim = 0;

    record_en = false;
    ready_rd = false;
    ready_fwriter = false;

    status.PortOpened = false;

    slotInitSimulator(1000, 512);
    timer_simulator->start(100);
}

Port::~Port()
{

    delete [] buf_simulator;
    delete [] data_simulator;
    delete [] timer_simulator;
    delete [] q_buffer;
    delete [] data_buf;

    qDebug("By in Thread!");
    emit sigFinishedPort();
}

void Port :: slotProcessPort(){
    qDebug("Hello from Port!");
    connect(&CPort,SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(slotHandleError(QSerialPort::SerialPortError)));
    connect(&CPort, SIGNAL(readyRead()),this,SLOT(slotReadInPort()));
    connect(timer_simulator, SIGNAL(timeout()), this, SLOT(slotTimerEv()));
}

void Port :: slotWriteSettingsPort(QString name, int baudrate,int DataBits,
                         int Parity,int StopBits, int FlowControl){
    SettingsPort.name = name;
    SettingsPort.baudRate = (QSerialPort::BaudRate) baudrate;
    SettingsPort.dataBits = (QSerialPort::DataBits) DataBits;
    SettingsPort.parity = (QSerialPort::Parity) Parity;
    SettingsPort.stopBits = (QSerialPort::StopBits) StopBits;
    SettingsPort.flowControl = (QSerialPort::FlowControl) FlowControl;

}

int Port :: slotConnectPort(void){//
    CPort.setPortName(SettingsPort.name);
    if (CPort.open(QIODevice::ReadWrite)) {

        if (CPort.setBaudRate(SettingsPort.baudRate)
             && CPort.setDataBits(SettingsPort.dataBits)
             && CPort.setParity(SettingsPort.parity)
             && CPort.setStopBits(SettingsPort.stopBits)
             && CPort.setFlowControl(SettingsPort.flowControl))
        {

            if (CPort.isOpen()){
                 status.PortOpened = true;
                 CPort.setReadBufferSize(nrcv*16); //!!! *2
                 CPort.clear();
                 sigPortOpened();
            }

        } else {
            CPort.close();
            sigError_(CPort.errorString().toLocal8Bit());
            status.PortOpened = false;
        }

    } else {
        CPort.close();
        sigError_(CPort.errorString().toLocal8Bit());
        status.PortOpened = false;
    }

    if(CPort.isOpen()) return 1;
    else return 0;
}

void Port::slotHandleError(QSerialPort::SerialPortError error)//
{
    if ( (CPort.isOpen()) && (error == QSerialPort::ResourceError)) {
        sigError_(CPort.errorString().toLocal8Bit());
        slotDisconnectPort();
    }
}//


void  Port::slotDisconnectPort(){
    if(CPort.isOpen()){
        CPort.close();
        sigError_(SettingsPort.name.toLocal8Bit() + " >> Закрыт!\r");
    }
}
//ot tuta kovuratji!!!
void Port :: slotWriteToPort(QByteArray data){
    if(CPort.isOpen()){
        CPort.write(data);
    }
    else{
       sigError_(CPort.errorString().toLocal8Bit());
    }
}


void Port :: slotReadInPort(){

   int port_data = CPort.bytesAvailable();

   if(port_data>=nrcv){

       //qDebug()<<"bytesAvailable Before:"<<CPort.bytesAvailable()<<endl;
       if(port_data%4) CPort.read(data_buf->data(), port_data%4);
       CPort.read(data_buf->data(), nrcv);
       //qDebug()<<"bytesAvailable After:"<<CPort.bytesAvailable()<<endl;

       status.status_num[status.Recieved]+=nrcv_t;
       if(record_en){
           q_buffer->enqueue(*data_buf);
           status.status_num[status.Writed_cash]+=nrcv_t;
           //qDebug()<<"Num data in queue:"<<q_buffer.size();
       }
       if((ready_rd)&&(out_recv_data!=0)){
           memcpy(out_recv_data, data_buf->data(), size_recv_data);
           status.status_num[status.Processed]+=nrcv_t;
           sigSendPortData();
           ready_rd = false;
       }
   }
   if((!q_buffer->isEmpty())&&(ready_fwriter)){
       ready_fwriter = false;
       status.status_num[status.Writed_file]+=nrcv_t;
       sigSendRecordData(q_buffer->dequeue());
   }
}

void Port :: slotSetReadyRead(char* data, int n){
   out_recv_data = data;
   size_recv_data = n;
   ready_rd = true;
}

void Port :: slotSetBufSize(int size){
   nrcv = size;
   data_buf->resize(nrcv);
}

void Port :: slotSetRecordEn(bool rn){
    record_en = rn;
}


void Port :: slotGetRecordData(){
   if(!q_buffer->isEmpty()){
       status.status_num[status.Writed_file]+=nrcv_t;
       sigSendRecordData(q_buffer->dequeue());
   }
   else ready_fwriter = true;
}

void Port::slotTimerEv(){
   if(fl_simulator){
       slotWriteToPort(*buf_simulator);
       //qDebug()<<"Timer On! Simulator On!: "<<cnt_sim++;
       status.status_num[status.Simulator]++;
   }
   else{
       //qDebug()<<"Timer On! Simulator Off!";
       status.status_num[status.Simulator] = -1;
   }
}

bool Port :: slotClearPort(){
   q_buffer->clear();

   if(CPort.clear()) status.PortOpened = true;
   else status.PortOpened = false;

   memset(status.status_num, 0, status.Max_num_Status*sizeof(int));
   time_timer.restart();

   return status.PortOpened;
}

void Port::slotGetStatus(){
   QStringList str_status;
   status.status_num[status.Time] = time_timer.elapsed()/1000;
   for(int i=0;i<status.Max_num_Status;i++)
      str_status.append(status.get_status(i));
   str_status[status.Time]+="sec";
   emit sigStatus(str_status.join(" "));
}

void Port::slotInitSimulator(int et, int nd){
   float d = (float)et;
   event_time = et; num_data = nd;
/*
   char cc[4];
   float d_out;
   memcpy(cc,reinterpret_cast<const char*>(&d), sizeof(d));
   memcpy(&d_out, cc, sizeof(cc));
   qDebug()<<"float out: "<<d_out<<endl;
*/

   for(int i=0;i<nd;i++) data_simulator[i] = d;
   data_simulator[30] = -1;

   buf_simulator->resize(nd*4);
   memcpy(buf_simulator->data(),(char*)data_simulator, nd*4);


}

void Port::slotSimulatorStart(int et, int nd){
   event_time = et; num_data = nd;
   fl_simulator = true;
}

void Port::slotSimulatorStop(){
   fl_simulator = false;
}

