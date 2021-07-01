#ifndef PORT_H
#define PORT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDataStream>
#include <QTimer>
#include <QTime>
#include <QQueue>
#include <QMessageBox>
#include <qbuffer.h>

#define DATA_TYPE      float

struct Settings {
    QString name;
    qint32 baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
};


class Status : public QObject
{
    Q_OBJECT

public:

    enum status_num{
        Recieved,
        Writed_cash,
        Processed,
        Writed_file,
        Time,
        Simulator,
        Max_num_Status
    };
    int status_num[Max_num_Status];

private:

    char lng;

    const QString status_list_ru[Max_num_Status] = {"Принято: ", ", Записано в кэш: ,", ", Обработано: ", ", Записано в файл: ", ", Время записи: ", ", Имитатор: "};
    const QString status_list_eng[Max_num_Status] = {"Recieved: ", ", Writted in cash: ", ", Processed: ", ", Writted in file: ", ", Writing Time: ", ", Simulator: "};


public:

    Status(QString lang = "ru");

    ~Status(){}

    QString get_status(int n);

    int set_lang(QString lang){
        if(lang == "ru"){ lng = 0; return 1; }
        else if(lang == "en"){ lng = 1; return 1; }
        return 0;
    }

};

class Port : public QObject
{
    Q_OBJECT

public:

    explicit Port(QObject *parent = 0);

    ~Port();

    QSerialPort CPort;
    Settings SettingsPort;
    QQueue<QByteArray>* q_buffer;

signals:

    void sigFinishedPort(); //
    void sigError_(QString err);
    void sigSendPortData(QByteArray);
    void sigSendRecordData(QByteArray);
    void sigStatus(QString);

public slots:

    void slotDisconnectPort();
    int  slotConnectPort(void);
    void slotWriteSettingsPort(QString name, int baudrate, int DataBits, int Parity, int StopBits, int FlowControl);
    void slotProcessPort();
    void slotWriteToPort(QByteArray data);
    void slotReadInPort();
    void slotSetReadyRead(char*, int);
    void slotSetRecordEn(bool);
    void slotGetRecordData();
    void slotSetBufSize(int size);
    void slotClearPort();
    void slotTimerEv();
    void slotSimulatorStart(int, int);
    void slotSimulatorStop();
    void slotInitSimulator(int, int);
    void slotGetStatus();

private slots:

    void slotHandleError(QSerialPort::SerialPortError error);//

public:
    int nrcv;
    int nrcv_t;
private:

    QByteArray* data_buf;

    char* out_recv_data;
    int size_recv_data;
    volatile bool ready_rd;

    volatile bool record_en;
    volatile bool ready_fwriter;

    QByteArray* buf_simulator;
    float data_simulator[8000];
    QTimer* timer_simulator;
    QTime time_timer;
    QDate d_time;
    int event_time;
    int num_data;
    bool fl_simulator;
    int cnt_sim;
    Status status;

};

#endif // PORT_H
