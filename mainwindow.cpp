#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QString>
#include <QDesktopWidget>
#include <QDesktopWidget>
#include <QScreen>
#include <QFileDialog>
#include <QMetaEnum>
#include <unistd.h>
#include <errno.h>
#include <QThread>
#include <QSettings>
#include "wadc_defines.h"
#include "beamunitlib.h"
#include "users_lang.h"


BeamUnitLib *beamunitlib;
Users_Lang *u_lng;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QVector<double> x(N_RCV_MAX), y(N_RCV_MAX);

    ui->setupUi(this);

    QString fileName = QCoreApplication::applicationDirPath()+"/settings.ini";

    beamunitlib = new BeamUnitLib();

    if (QFile::exists(fileName)) {
         QSettings settings(fileName, QSettings::IniFormat);

         ui->PortNameEdit->setText(settings.value("SETTINGS/COM_PORT", "COM5").toString());
         ui->lineEdit->setText(settings.value("SETTINGS/porog", "0.4").toString());
         ui->spinBox_2->setValue(settings.value("SETTINGS/time_mean", "1000").toInt());
         ui->lineEdit_14->setText(settings.value("SETTINGS/koef_usil", "0.0008056641").toString());
         ui->lineEdit_15->setText(settings.value("SETTINGS/moment_sopr", "0.0008056641").toString());
         ui->lineEdit_12->setText(settings.value("SETTINGS/kpd_reduktora", "700000").toString());
         ui->lineEdit_13->setText(settings.value("SETTINGS/pered_otnoshenie", "0.0008056641").toString());
         beamunitlib->Fd_str = settings.value("SETTINGS/Fd", "3500").toString();
         beamunitlib->Volts_inv_str = settings.value("SETTINGS/1_div_Volts", "0.0008056641").toString();

//         ui->lineEdit_2->setText(settings.value("SETTINGS/Fd", "3500").toString()); //DEL
//         ui->lineEdit_5->setText(settings.value("SETTINGS/1_div_Volts", "0.0008056641").toString());

    }


    for(int i = 0; i < beamunitlib->data_mng->N_RCV; i++)
    {
       x[i] = i;
       y[i] = -4.0+(double)i*(8.0/(double)beamunitlib->data_mng->N_RCV);
    }




/*
    ui->PortNameBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->PortNameBox->addItem(info.portName());
    }
*/

    ui->Button_Start_Stop->setEnabled(false);
    ui->groupBox->setVisible(false);
//----------- настройка QCustomPlot -----------------------------------------------
    ui->Plot1->addGraph();
    ui->Plot2->addGraph();

    ui->Plot1->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->Plot1->graph(0)->setData(x,y);
    ui->Plot2->graph(0)->setData(x,y);
    ui->Plot1->xAxis->setRange(0, beamunitlib->data_mng->N_RCV);
    ui->Plot1->yAxis->setRange(-2.0, 2.0);
    ui->Plot2->xAxis->setRange(0, beamunitlib->data_mng->N_RCV);
    ui->Plot2->yAxis->setRange(-2.0, 2.0);
    ui->Plot1->replot();
    ui->Plot2->replot();

    ui->lineEdit_3->setText("2");
    ui->lineEdit_4->setText("-2");
    ui->lineEdit_8->setText("2");
    ui->lineEdit_9->setText("-2");

    ui->label_19->setText("Отключено");
    ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: black;");
    ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgray; }");

    ui->label_tah->setStyleSheet("QLabel { background-color : yellow; }");

    beamunitlib->data_mng->k_norm[0] = beamunitlib->Volts_inv_str.toDouble(); //ui->lineEdit_5->text().toDouble();

    beamunitlib->torsi_cntrl->Tahometr_On = true;
    beamunitlib->data_mng->Fd_Inv = 1.0/beamunitlib->Fd_str.toDouble();  //ui->lineEdit_2->text().toDouble();
    beamunitlib->torsi_cntrl->TimeAverTah = ui->spinBox_2->text().toDouble();
    beamunitlib->torsi_cntrl->PorogTah = ui->lineEdit->text().toDouble();
    beamunitlib->torsi_cntrl->cnt_time_0_before = 0.00001;

    if(ui->checkBox->isChecked()==true){
        ui->label_2->setText("Зап в файл Вкл!");
        ui->label_2->setStyleSheet("font-weight: bold; font-size: 10pt; color: green;");
        beamunitlib->data_mng->fl_write_to_file = true;
    }else{
        ui->label_2->setText("Зап в файл Откл!");
        ui->label_2->setStyleSheet("font-weight: bold; font-size: 10pt; color: red;");
        beamunitlib->data_mng->fl_write_to_file = false;
    }
//---------------------------------------------------------------------------------

//------------- запуск и настройка потока с классом COM порта ---------------------

    QThread *thread_New = new QThread;//Создаем поток для порта платы

    Port *PortNew = new Port();//Создаем обьект по классу
    PortNew->moveToThread(thread_New);//помешаем класс  в поток
    PortNew->thisPort.moveToThread(thread_New);//Помещаем сам порт в поток
    connect(PortNew, SIGNAL(error_(QString)), this, SLOT(Print(QString)));//Лог ошибок
    connect(thread_New, SIGNAL(started()), PortNew, SLOT(process_Port()));//Переназначения метода run
    connect(PortNew, SIGNAL(finished_Port()), thread_New, SLOT(quit()));//Переназначение метода выход
    connect(thread_New, SIGNAL(finished()), PortNew, SLOT(deleteLater()));//Удалить к чертям поток
    connect(PortNew, SIGNAL(finished_Port()), thread_New, SLOT(deleteLater()));//Удалить к чертям поток
    connect(this,SIGNAL(savesettings(QString,int,int,int,int,int)),PortNew,SLOT(Write_Settings_Port(QString,int,int,int,int,int)));//Слот - ввод настроек!
    connect(this,SIGNAL(Signal_SetRunRecv(bool Recv_En)),PortNew,SLOT(SetRunRecv(bool Recv_En)));
    connect(this,SIGNAL(Signal_SetRunReg(bool Reg_En)),PortNew,SLOT(SetRunReg(bool Reg_En)));
    connect(this, SIGNAL(Connect_Port()),PortNew,SLOT(ConnectPort()));//по нажатию кнопки подключить порт
    connect(this, SIGNAL(Disconnect_Port()),PortNew,SLOT(DisconnectPort()));//по нажатию кнопки отключить порт
    connect(PortNew, SIGNAL(outPort(QByteArray)), this, SLOT(ReadFromPort(QByteArray)));//вывод в текстовое поле считанных данных
    connect(this,SIGNAL(writeData(QByteArray)),PortNew,SLOT(WriteToPort(QByteArray)));//отправка в порт данных


    PortNew->nrcv = beamunitlib->data_mng->N_RCV;

    thread_New->start(QThread::LowestPriority);

    //MyServer server;

    MyServer *server = new MyServer();
    QThread *thread_tcp = new QThread;//Создаем поток для порта платы
    server->moveToThread(thread_tcp);
    server->server->moveToThread(thread_tcp);
    //connect(server,SIGNAL(sig_readyRead(QByteArray)),this,SLOT(ReadFromTCP(QByteArray)));
    connect(server,SIGNAL(sig_readyRead(QByteArray)),this,SLOT(ReadFromPort(QByteArray)));
    connect(server,SIGNAL(sig_Connect_Socket()),this,SLOT(connect_recv()));
    connect(server,SIGNAL(sig_DisConnect_Socket()),this,SLOT(disconnect_recv()));

    beamunitlib->data_mng->cnt_reg_recv = 0;
    beamunitlib->data_mng->cnt_recv = 0;
    beamunitlib->data_mng->cnt_data_recv = 0;
    beamunitlib->data_mng->Fl_Start = true;
    beamunitlib->torsi_cntrl->CntObTah = 0;
    beamunitlib->torsi_cntrl->Pgd_sredn = 0;
    beamunitlib->torsi_cntrl->Pval_sredn = 0;
    beamunitlib->torsi_cntrl->cnt_p_sredn = 0;
   // ui->Button_Start_Stop->setText("Стоп");
   // ui->Button_Connect->setEnabled(false);

    thread_tcp->start();


    //server->


//---------------------------------------------------------------------------------

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::disconnect_recv(){

    //qDebug()<<"disconnect_recv";
    ui->label_19->setText("Отключено");

}

void MainWindow::connect_recv(){

   //qDebug()<<"connect_recv";
   ui->label_19->setText("Подключено");

}

void MainWindow::on_Button_Connect_clicked()
{
    QByteArray data_cdg;
    QString file1;
    int i;
    short* ptr_reg_data;
    short sample = 0;
    double fsample = 0.0;
    QFile file;

    if(beamunitlib->data_mng->Fl_Connect==false){
        //savesettings(ui->PortNameBox->currentText(),COM_BaudRate,8,0,1,0);
        savesettings(ui->PortNameEdit->text(),COM_BaudRate,8,0,1,0);
        beamunitlib->data_mng->Fl_Connect = true;
        ui->Button_Connect->setText("Отключить");
        ui->Button_Start_Stop->setEnabled(true);
        Connect_Port();
//        ui->listWidget->addItem("Подключение к порту");

        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");

        for(i=0;i<1000000;i++){i=i;}

    }else{
        beamunitlib->data_mng->Fl_Connect = false;
        ui->Button_Connect->setText("Подключить");
        ui->Button_Start_Stop->setEnabled(false);
        Disconnect_Port();
//        ui->listWidget->addItem("Отключение от порта");
        ui->label_19->setText("Отключено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: black;");

        for(i=0;i<1000000;i++){i=i;}
    }
    Signal_SetRunRecv(beamunitlib->data_mng->Fl_Connect);
}

void MainWindow::on_Button_Start_Stop_clicked()
{
  QByteArray data_cdg;
  unsigned int i;

    if(beamunitlib->data_mng->Fl_Start==false){

        beamunitlib->reg_buf.clear();
        beamunitlib->data_mng->cnt_reg_recv = 0;
        beamunitlib->data_mng->cnt_recv = 0;
        beamunitlib->data_mng->cnt_data_recv = 0;
        beamunitlib->data_mng->Fl_Start = true;
        beamunitlib->torsi_cntrl->CntObTah = 0;
        beamunitlib->torsi_cntrl->Pgd_sredn = 0;
        beamunitlib->torsi_cntrl->Pval_sredn = 0;
        beamunitlib->torsi_cntrl->cnt_p_sredn = 0;

        ui->Button_Start_Stop->setText("Стоп");

        ui->Button_Connect->setEnabled(false);

        if(ui->checkBox->isChecked()){
            beamunitlib->filereg = QFileDialog::getSaveFileName(0, "Сохрание данных регистрации", "");
            if(beamunitlib->filereg.length()>0) beamunitlib->data_mng->Reg_On=true;
            ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgreen; }");
        }

        ui->checkBox->setEnabled(false);

        data_cdg.append(Conf_Byte_Start);
        writeData(data_cdg);
        data_cdg.clear();

//        ui->listWidget->addItem("Прием данных с АЦП запущен");

    }else{

        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
        beamunitlib->data_mng->Fl_Start = false;
        ui->Button_Start_Stop->setText("Старт");
        ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgray; }");
        ui->Button_Connect->setEnabled(true);

        data_cdg.append(Conf_Byte_Stop);
        writeData(data_cdg);
        data_cdg.clear();
        for(i=0;i<100000;i++){i=i;}


        //write reg data
        if(ui->checkBox->isChecked()) beamunitlib->write_to_file_reg_data(0);

        beamunitlib->data_mng->Reg_On=false;
        beamunitlib->data_mng->cnt_reg_recv = 0;
        ui->checkBox->setEnabled(true);

//        ui->listWidget->addItem("Прием данных с АЦП остановлен");

    }
}


void MainWindow::ReadFromPort(QByteArray data)
{
    unsigned int i;
    QVector<double> x(N_RCV_MAX*2), y(N_RCV_MAX*2);
    QString str;
    double mx,mn;

    qDebug()<<"ReadFromPort - "<<data.count();


    if(beamunitlib->data_mng->Fl_Start){
        ui->label_19->setText("Исправно");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: green;");
    }
    else{
        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
    }


//handling!!!!
    beamunitlib->Handler_unit(data.data(),data.count());


    if(beamunitlib->data_mng->cnt_cycle_view==beamunitlib->data_mng->Refresh_Cycle_View){


        if(beamunitlib->torsi_cntrl->CntObTah==0) ui->label_tah->setText("--------");
        else   ui->label_tah->setText(QString::number(beamunitlib->torsi_cntrl->ObMinTah_Aver));

        if(beamunitlib->torsi_cntrl->Fl_mean_set){
            str = "Средн До: " + QString::number(beamunitlib->torsi_cntrl->Sum0Before);
            ui->label_13->setText(str);

            str = "Средн После: " + QString::number(beamunitlib->torsi_cntrl->Sum0Before);
            ui->label_9->setText(str);

            ui->pushButton_2->setEnabled(true);
            ui->pushButton_7->setEnabled(true);

            beamunitlib->torsi_cntrl->Fl_mean_set = false;
        }

        str = "данных: " + QString::number(beamunitlib->data_mng->cnt_data_recv/1000) + "к / "+ "время: " + QString::number(beamunitlib->data_mng->cnt_data_recv*beamunitlib->data_mng->Fd_Inv) + " c";
        ui->groupBox_13->setTitle(str);

        str = "М, вал: " + QString::number(beamunitlib->torsi_cntrl->M_val);
        ui->label_7->setText(str);
        str = "Р ГД, кВт: " + QString::number(beamunitlib->torsi_cntrl->Pgd);
        ui->label_15->setText(str);
        str = "Р ВАЛ, кВт: " + QString::number(beamunitlib->torsi_cntrl->Pval);
        ui->label_14->setText(str);

        str = "Рср ГД, кВт: " + QString::number(beamunitlib->torsi_cntrl->Pgd_sredn/(double)beamunitlib->torsi_cntrl->cnt_p_sredn);
        ui->label_20->setText(str);
        str = "Рср ВАЛ, кВт: " + QString::number(beamunitlib->torsi_cntrl->Pval_sredn/(double)beamunitlib->torsi_cntrl->cnt_p_sredn);
        ui->label_21->setText(str);

        str = "Тахометр, об: " + QString::number(beamunitlib->torsi_cntrl->CntObTah);
        ui->groupBox_7->setTitle(str);

        beamunitlib->data_mng->cnt_cycle_view = 0;

        //ui->Plot1->xAxis->setRange(0, num_block_view);
        //ui->Plot2->xAxis->setRange(0, num_block_view);

            //view_ch_1
            mx = beamunitlib->data_mng->obr_buf_f[0][i];
            mn = beamunitlib->data_mng->obr_buf_f[0][i];
            for(int i = 0; i < beamunitlib->data_mng->num_block_view; i++){
               x[i] = i;
               y[i] = beamunitlib->data_mng->obr_buf_f[0][i];
               if(mx<y[i]) mx = y[i];
               if(mn>y[i]) mn = y[i];
            }

            beamunitlib->data_mng->mx_ch[0]=mx;
            beamunitlib->data_mng->min_ch[0]=mn;

            ui->Plot1->graph(0)->setData(x,y);
            ui->Plot1->replot();

            //view_ch_2
            mx = beamunitlib->data_mng->obr_buf_f[1][i];
            mn = beamunitlib->data_mng->obr_buf_f[1][i];
            for(int i = 0; i < beamunitlib->data_mng->num_block_view; i++){
               x[i] = i;
               y[i] = beamunitlib->data_mng->obr_buf_f[1][i];
               if(mx<y[i]) mx = y[i];
               if(mn>y[i]) mn = y[i];
            }

            beamunitlib->data_mng->mx_ch[1]=mx;
            beamunitlib->data_mng->min_ch[1]=mn;

            ui->Plot2->graph(0)->setData(x,y);
            ui->Plot2->replot();

    }//end  if(cnt_cycle_view==Refresh_Cycle_View)



}


void MainWindow::on_pushButton_7_clicked()
{
    beamunitlib->torsi_cntrl->Fl_0_Before = 1;
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
}

void MainWindow::on_lineEdit_returnPressed()
{
    beamunitlib->torsi_cntrl->PorogTah = ui->lineEdit->text().toDouble();
}

void MainWindow::Write_byte_to_serial_port(char cb)
{
  QByteArray data_cdg;

    data_cdg.append(cb);
    writeData(data_cdg);
    data_cdg.clear();

}


void MainWindow::on_lineEdit_3_returnPressed()
{
    double Ym_1;
    double Ym_2;
    Ym_1 = ui->lineEdit_3->text().toDouble();
    Ym_2 = ui->lineEdit_4->text().toDouble();
    ui->Plot1->yAxis->setRange(Ym_2, Ym_1);
    ui->Plot1->replot();
}

void MainWindow::on_lineEdit_4_returnPressed()
{
    double Ym_1;
    double Ym_2;
    Ym_1 = ui->lineEdit_3->text().toDouble();
    Ym_2 = ui->lineEdit_4->text().toDouble();
    ui->Plot1->yAxis->setRange(Ym_2, Ym_1);
    ui->Plot1->replot();
}

//*******************************************************************************
//Функция выполняет запись в файл
int write_in_file(char *the_data, unsigned int the_size){
    QString str = QFileDialog::getSaveFileName(0, "Файл для записи");

    if(str == ""){
        return 1;
    }

    QFile file;
    file.setFileName(str);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        return 2;
    }

    file.write(the_data, the_size);
    file.close();

    return 0;
}

/* !!! DEL
void MainWindow::on_lineEdit_5_returnPressed()
{
    k_norm = ui->lineEdit_5->text().toDouble();
}
*/

void MainWindow::on_lineEdit_6_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_6->text().toDouble();
    Xm_2 = ui->lineEdit_7->text().toDouble();
    ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot1->replot();
}

void MainWindow::on_lineEdit_7_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_6->text().toDouble();
    Xm_2 = ui->lineEdit_7->text().toDouble();
    ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot1->replot();
}

void MainWindow::on_pushButton_3_clicked()
{
    double Ym_1;
    double Ym_2;

    if(beamunitlib->data_mng->cnt_recv>0){

        Ym_1 = beamunitlib->data_mng->mx_ch[0]+beamunitlib->data_mng->mx_ch[0]*0.3;
        Ym_2 = beamunitlib->data_mng->min_ch[0]-beamunitlib->data_mng->min_ch[0]*0.3;
        ui->Plot1->yAxis->setRange(Ym_2, Ym_1);
        ui->Plot1->replot();

        ui->lineEdit_3->setText(QString::number(Ym_1));
        ui->lineEdit_4->setText(QString::number(Ym_2));

    }
}

void MainWindow::on_pushButton_8_clicked()
{
        ui->Plot1->xAxis->setRange(0, 2000);
        ui->Plot1->replot();
        ui->lineEdit_6->setText("0");
        ui->lineEdit_7->setText("2000");
}

void MainWindow::on_lineEdit_8_returnPressed()
{
    double Ym_1;
    double Ym_2;
    Ym_1 = ui->lineEdit_8->text().toDouble();
    Ym_2 = ui->lineEdit_9->text().toDouble();
    ui->Plot2->yAxis->setRange(Ym_2, Ym_1);
    ui->Plot2->replot();
}

void MainWindow::on_lineEdit_9_returnPressed()
{
    double Ym_1;
    double Ym_2;
    Ym_1 = ui->lineEdit_8->text().toDouble();
    Ym_2 = ui->lineEdit_9->text().toDouble();
    ui->Plot2->yAxis->setRange(Ym_2, Ym_1);
    ui->Plot2->replot();
}

void MainWindow::on_lineEdit_10_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_10->text().toDouble();
    Xm_2 = ui->lineEdit_11->text().toDouble();
    ui->Plot2->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot2->replot();
}

void MainWindow::on_lineEdit_11_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_10->text().toDouble();
    Xm_2 = ui->lineEdit_11->text().toDouble();
    ui->Plot2->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot2->replot();
}

void MainWindow::on_pushButton_9_clicked()
{
    ui->Plot2->xAxis->setRange(0, 2000);
    ui->Plot2->replot();
    ui->lineEdit_10->setText("0");
    ui->lineEdit_11->setText("2000");
}

void MainWindow::on_pushButton_6_clicked()
{
    double Ym_1;
    double Ym_2;

    if(beamunitlib->data_mng->cnt_recv>0){

        Ym_1 = beamunitlib->data_mng->mx_ch[1]+beamunitlib->data_mng->mx_ch[1]*0.3;
        Ym_2 = beamunitlib->data_mng->min_ch[1]-beamunitlib->data_mng->min_ch[1]*0.3;
        ui->Plot2->yAxis->setRange(Ym_2, Ym_1);
        ui->Plot2->replot();

        ui->lineEdit_8->setText(QString::number(Ym_1));
        ui->lineEdit_9->setText(QString::number(Ym_2));

    }
}

void MainWindow::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked()==true){
        ui->label_2->setText("Зап в файл Вкл!");
        ui->label_2->setStyleSheet("font-weight: bold; font-size: 10pt; color: green;");
        beamunitlib->data_mng->fl_write_to_file = true;
    }else{
        ui->label_2->setText("Зап в файл Откл!");
        ui->label_2->setStyleSheet("font-weight: bold; font-size: 10pt; color: red");
        beamunitlib->data_mng->fl_write_to_file = false;
    }
}


void MainWindow::on_spinBox_2_valueChanged(const QString &arg1)
{
    beamunitlib->torsi_cntrl->TimeAverTah = ui->spinBox_2->text().toDouble();
}

void MainWindow::on_pushButton_2_clicked()
{
    beamunitlib->torsi_cntrl->Fl_0_After = 1;
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
}

void MainWindow::on_lineEdit_14_returnPressed()
{
    beamunitlib->torsi_cntrl->Kusil = ui->lineEdit_14->text().toDouble();
}

void MainWindow::on_lineEdit_15_returnPressed()
{
    beamunitlib->torsi_cntrl->Msopr = ui->lineEdit_15->text().toDouble();
}

void MainWindow::on_lineEdit_12_returnPressed()
{
    beamunitlib->torsi_cntrl->KPD_Red = ui->lineEdit_12->text().toDouble()/100.0;
}

void MainWindow::on_lineEdit_13_returnPressed()
{
    beamunitlib->torsi_cntrl->Per_Otnosh = ui->lineEdit_13->text().toDouble()/100.0;
}

void MainWindow::on_lineEdit_13_editingFinished()
{
    beamunitlib->torsi_cntrl->Per_Otnosh = ui->lineEdit_13->text().toDouble()/100.0;
}

void MainWindow::on_lineEdit_12_editingFinished()
{
    beamunitlib->torsi_cntrl->KPD_Red = ui->lineEdit_12->text().toDouble()/100.0;
}

void MainWindow::on_lineEdit_15_editingFinished()
{
   beamunitlib->torsi_cntrl->Msopr = ui->lineEdit_15->text().toDouble();
}

void MainWindow::on_lineEdit_14_editingFinished()
{
  beamunitlib->torsi_cntrl->Kusil = ui->lineEdit_14->text().toDouble();
}

void MainWindow::on_lineEdit_editingFinished()
{
   beamunitlib->torsi_cntrl->PorogTah = ui->lineEdit->text().toDouble();
}


void MainWindow::on_checkBox_2_stateChanged(int arg1)
{

}


void MainWindow::closeEvent(QCloseEvent *event)
{
QString fileName = QCoreApplication::applicationDirPath()+"/settings.ini";

    QSettings settings(fileName,QSettings::IniFormat);

    settings.setValue("SETTINGS/COM_PORT",ui->PortNameEdit->text());
    settings.setValue("SETTINGS/porog",ui->lineEdit->text());
    settings.setValue("SETTINGS/time_mean",ui->spinBox_2->value());
    settings.setValue("SETTINGS/koef_usil",ui->lineEdit_14->text());
    settings.setValue("SETTINGS/moment_sopr",ui->lineEdit_15->text());
    settings.setValue("SETTINGS/kpd_reduktora",ui->lineEdit_12->text());
    settings.setValue("SETTINGS/pered_otnoshenie",ui->lineEdit_13->text());
//    settings.setValue("SETTINGS/Fd",ui->lineEdit_2->text()); !!! DEL
//    settings.setValue("SETTINGS/1_div_Volts",ui->lineEdit_5->text());  !!! DEL

    settings.sync(); //записываем настройки

}
