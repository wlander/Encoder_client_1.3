#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>
#include <QDataStream>

#include "port.h"
#include "myudp.h"
#include "mytcp.h"
#include "wadc_defines.h"
#include "beamunitlib.h"
#include "users_lang.h"
#include "fwriter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    Port *PortC;
    BeamUnitLib *beamunitlib;
    Users_Lang *u_lng;
    fwriter *fwrtr;

signals:
    void savesettings(QString name, int baudrate, int DataBits, int Parity, int StopBits, int FlowControl);
    int Connect_Port();
    void Disconnect_Port();
    void writeData(QByteArray);
    void ReadData(QByteArray);
    void SigReadyRecv(char*, int);
    void Signal_SetRunReg(bool);
    void Signal_Runfwrite();
    void sig_ask_status();
    void sigPortClear();
    void sig_run_simulator(int, int);
    void sig_set_simulator(int,int);
    void sig_stop_simulator();

private slots:
    void on_Button_Connect_clicked();
    void on_Button_Start_Stop_clicked();
    void ReadFromPort(QByteArray);
    void disconnect_recv();
    void connect_recv();
    void slot_recv_status(QString);
    void  slotPlotReplot2();
    void  slotPlotReplot1();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_lineEdit_returnPressed();

    void on_lineEdit_3_returnPressed();

    void on_lineEdit_4_returnPressed();

    void on_lineEdit_6_returnPressed();

    void on_lineEdit_7_returnPressed();

    void on_pushButton_3_clicked();

    void on_pushButton_8_clicked();

    void on_lineEdit_8_returnPressed();

    void on_lineEdit_9_returnPressed();

    void on_lineEdit_10_returnPressed();

    void on_lineEdit_11_returnPressed();

    void on_pushButton_9_clicked();

    void on_spinBox_2_valueChanged(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_lineEdit_14_returnPressed();

    void on_lineEdit_15_returnPressed();

    void on_lineEdit_12_returnPressed();

    void on_lineEdit_13_returnPressed();

    void on_lineEdit_13_editingFinished();

    void on_lineEdit_12_editingFinished();

    void on_lineEdit_15_editingFinished();

    void on_lineEdit_14_editingFinished();

    void on_lineEdit_editingFinished();

    void on_checkBox_2_stateChanged(int arg1);

    void on_pushButton_clicked();

    void on_checkBox_wr_stateChanged(int arg1);

    void on_lineEdit_2_editingFinished();

private:
    Ui::MainWindow *ui;
    void Write_byte_to_serial_port(char cb);
};

#endif // MAINWINDOW_H
