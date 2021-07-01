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
#include "fwriter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);  

    ui->lineEdit_3->setText("2");
    ui->lineEdit_4->setText("-2");
    ui->lineEdit_8->setText("2");
    ui->lineEdit_9->setText("-2");
/*
    ui->lineEdit_3->setVisible(false);
    ui->lineEdit_4->setVisible(false);
    ui->lineEdit_8->setVisible(false);
    ui->lineEdit_9->setVisible(false);
*/
    ui->label_19->setText("Отключено");
    ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: black;");
    ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgray; }");

    ui->label_tah->setStyleSheet("QLabel { background-color : yellow; }");

    ui->checkBox_proc->setChecked(true);
    ui->checkBox_wr->setChecked(true);

    ui->groupBox_2->setVisible(false);
    ui->checkBox_2->setVisible(false);
    ui->pushButton_2->setVisible(false);
    ui->pushButton_7->setVisible(false);
    ui->label_13->setVisible(false);
    ui->label_9->setVisible(false);

//-------------------- Custom manage and processing class -----------------------------------------------------
    beamunitlib = new BeamUnitLib();

    beamunitlib->data_mng->k_norm[0] = beamunitlib->Volts_inv_str.toDouble(); //ui->lineEdit_5->text().toDouble();
    beamunitlib->torsi_cntrl->Tahometr_On = true;
    beamunitlib->data_mng->Fd_Inv = 1.0/beamunitlib->Fd_str.toDouble();  //ui->lineEdit_2->text().toDouble();
    beamunitlib->torsi_cntrl->TimeAverTah = ui->spinBox_2->text().toDouble();
    beamunitlib->torsi_cntrl->PorogTah = ui->lineEdit->text().toDouble();
    beamunitlib->torsi_cntrl->cnt_time_0_before = 0.00001;
    beamunitlib->torsi_cntrl->Fl_simulator = false;

    beamunitlib->data_mng->cnt_reg_recv = 0;
    beamunitlib->data_mng->cnt_recv = 0;
    beamunitlib->data_mng->cnt_data_recv = 0;
    beamunitlib->data_mng->Fl_Start = false;
    beamunitlib->torsi_cntrl->CntObTah = 0;
    beamunitlib->torsi_cntrl->Pgd_sredn = 0;
    beamunitlib->torsi_cntrl->Pval_sredn = 0;
    beamunitlib->torsi_cntrl->cnt_p_sredn = 0;

//----------- настройка QCustomPlot -----------------------------------------------
        ui->Plot1->addGraph();
        ui->Plot2->addGraph();

        ui->Plot1->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
        ui->Plot2->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

        ui->Plot1->xAxis->rescale(true);
        ui->Plot2->xAxis->rescale(true);


        ui->Plot1->graph(0)->setData(beamunitlib->x,beamunitlib->y);
        ui->Plot2->graph(0)->setData(beamunitlib->x,beamunitlib->y);

        ui->Plot1->xAxis->setRange(0, beamunitlib->data_mng->N_RCV);
        ui->Plot1->yAxis->setRange(-2.0, 2.0);
        ui->Plot2->xAxis->setRange(0, beamunitlib->data_mng->N_RCV);
        ui->Plot2->yAxis->setRange(-2.0, 2.0);
        ui->Plot1->replot();
        ui->Plot2->replot();
        ui->Plot1->setInteraction(QCP::iRangeDrag, true);
        ui->Plot1->setInteraction(QCP::iRangeZoom, true);
        ui->Plot1->setInteraction(QCP::iSelectPlottables, true);
        connect(ui->Plot1, SIGNAL(afterReplot()), this, SLOT(slotPlotReplot2()));
        ui->Plot2->setInteraction(QCP::iRangeDrag, true);
        ui->Plot2->setInteraction(QCP::iRangeZoom, true);
        ui->Plot2->setInteraction(QCP::iSelectPlottables, true);
        connect(ui->Plot2, SIGNAL(afterReplot()), this, SLOT(slotPlotReplot1()));//Лог ошибок
//---------------------------------------------------------------------------------
        QString fileName = QCoreApplication::applicationDirPath()+"/settings.ini";
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
        }
//------------- запуск и настройка потока с классом COM порта ---------------------

    QThread *thread_port = new QThread;//Создаем поток для порта платы
    PortC = new Port();//Создаем обьект по классу

    PortC->moveToThread(thread_port);//помешаем класс  в поток
    PortC->CPort.moveToThread(thread_port);//Помещаем сам порт в поток
    connect(PortC, SIGNAL(sigError_(QString)), this, SLOT(Print(QString)));//Лог ошибок
    connect(thread_port, SIGNAL(started()), PortC, SLOT(slotProcessPort()));//Переназначения метода run
    connect(PortC, SIGNAL(sigFinishedPort()), thread_port, SLOT(quit()));//Переназначение метода выход
    connect(thread_port, SIGNAL(finished()), PortC, SLOT(deleteLater()));//Удалить к чертям поток
    connect(PortC, SIGNAL(sigFinishedPort()), thread_port, SLOT(deleteLater()));//Удалить к чертям поток
    connect(this,SIGNAL(savesettings(QString,int,int,int,int,int)),PortC,SLOT(slotWriteSettingsPort(QString,int,int,int,int,int)));//Слот - ввод настроек!
    connect(this,SIGNAL(SigReadyRecv(char*, int)),PortC,SLOT(slotSetReadyRead(char*, int)));
    connect(this,SIGNAL(Signal_SetRunReg(bool)),PortC,SLOT(slotSetRecordEn(bool)));
    connect(this, SIGNAL(Connect_Port()),PortC,SLOT(slotConnectPort()));//по нажатию кнопки подключить порт
    connect(this, SIGNAL(Disconnect_Port()),PortC,SLOT(slotDisconnectPort()));//по нажатию кнопки отключить порт
    connect(PortC, SIGNAL(sigSendPortData(QByteArray)), this, SLOT(ReadFromPort(QByteArray)));//вывод в текстовое поле считанных данных
    connect(this,SIGNAL(writeData(QByteArray)),PortC,SLOT(slotWriteToPort(QByteArray)));//отправка в порт данных
    connect(this, SIGNAL(sig_ask_status()), PortC, SLOT(slotGetStatus()));
    connect(PortC, SIGNAL(sigStatus(QString)), this, SLOT(slot_recv_status(QString)));
    connect(this, SIGNAL(sig_stop_simulator()),PortC,SLOT(slotSimulatorStop()));//
    connect(this, SIGNAL(sig_run_simulator(int, int)),PortC,SLOT(slotSimulatorStart(int, int)));//
    connect(this, SIGNAL(sig_set_simulator(int,int)), PortC, SLOT(slotInitSimulator(int,int)));
    connect(this, SIGNAL(sigPortClear()), PortC, SLOT(slotClearPort()));

    PortC->slotSetBufSize(beamunitlib->data_mng->N_RCV*4);

    //MyServer server;
/*
    MyServer *server = new MyServer();
    QThread *thread_tcp = new QThread;//Создаем поток для порта платы
    server->moveToThread(thread_tcp);
    server->server->moveToThread(thread_tcp);
    connect(server,SIGNAL(sig_readyRead(QByteArray)),this,SLOT(ReadFromPort(QByteArray)));
    connect(server,SIGNAL(sig_Connect_Socket()),this,SLOT(connect_recv()));
    connect(server,SIGNAL(sig_DisConnect_Socket()),this,SLOT(disconnect_recv()));
*/


    fwrtr = new fwriter();
    QThread *thread_filewr = new QThread;//
    fwrtr->moveToThread(thread_filewr);

    connect(PortC, SIGNAL(sigSendRecordData(QByteArray)), fwrtr, SLOT(proc(QByteArray)));
    connect(fwrtr, SIGNAL(fwriter_ready()), PortC, SLOT(slotGetRecordData()));

    thread_filewr->start();

//    thread_tcp->start();
    thread_port->start();


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
    QFile file;
    QMessageBox msgBox;

    if(beamunitlib->data_mng->Fl_Connect==false){

        savesettings(ui->PortNameEdit->text(),COM_BaudRate,8,0,1,0);
        Connect_Port();

 //       if(PortNew->thisPort.isOpen()){

            beamunitlib->data_mng->Fl_Connect = true;
            ui->Button_Connect->setText("Отключить");
            ui->Button_Start_Stop->setEnabled(true);
    //      ui->listWidget->addItem("Подключение к порту");
            ui->label_19->setText("Остановлено");
            ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
            for(i=0;i<1000000;i++){i=i;}

 //       }
 /*       else{
            msgBox.setText("Ошибка подключения!");
            msgBox.exec();
        }
*/
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

}

void MainWindow::on_Button_Start_Stop_clicked()
{
  unsigned int i;
  QByteArray data_cdg;
  QString str = "";
  QMessageBox msgBox;


    if(beamunitlib->data_mng->Fl_Start==false){

        if(ui->checkBox_wr->isChecked()){
            str = QFileDialog::getSaveFileName(0, "Файл для записи");
            if(str!=""){
               fwrtr->set_file(str);
               emit sigPortClear();
               emit Signal_SetRunReg(true);
            }
            else{
                emit Signal_SetRunReg(false);
                msgBox.setText("Файл не выбран!");
                msgBox.exec();
                return;
            }
        }
        else{
            emit Signal_SetRunReg(false);
        }

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

        if(ui->checkBox_wr->isChecked()){
            ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgreen; }");
        }

        ui->checkBox_wr->setEnabled(false);

        SigReadyRecv(beamunitlib->ptr_data_recv, beamunitlib->data_mng->N_RCV*4);

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

        ui->checkBox_wr->setEnabled(true);

//        ui->listWidget->addItem("Прием данных с АЦП остановлен");

    }
}


void MainWindow::ReadFromPort(QByteArray data)
{
    size_t i;
    QString str;

    //qDebug()<<"ReadFromPort - "<<data.count();


    if(beamunitlib->data_mng->Fl_Start){
        ui->label_19->setText("Исправно");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: green;");
    }
    else{
        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
    }


//handling!!!!
    if(ui->checkBox_proc->isChecked()) beamunitlib->Handler_unit();

    if((beamunitlib->data_mng->cnt_cycle_view==beamunitlib->data_mng->Refresh_Cycle_View) && (ui->checkBox_proc->isChecked())){


        if(beamunitlib->torsi_cntrl->CntObTah==0) ui->label_tah->setText("--------");
        else   ui->label_tah->setText(QString::number(beamunitlib->torsi_cntrl->ObMinTah_Aver));

/*luch5
        if(beamunitlib->torsi_cntrl->Fl_mean_set){

            str = "Средн До: " + QString::number(beamunitlib->torsi_cntrl->Sum0Before);
            ui->label_13->setText(str);

            str = "Средн После: " + QString::number(beamunitlib->torsi_cntrl->Sum0Before);
            ui->label_9->setText(str);

            ui->pushButton_2->setEnabled(true);
            ui->pushButton_7->setEnabled(true);

            beamunitlib->torsi_cntrl->Fl_mean_set = false;
        }

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
luch5*/

        str = "Тахометр, об: " + QString::number(beamunitlib->torsi_cntrl->CntObTah);
        ui->groupBox_7->setTitle(str);

        beamunitlib->data_mng->cnt_cycle_view = 0;


         //view_ch_1,2
         beamunitlib->set_data_show(0);

         ui->Plot1->graph(0)->setData(beamunitlib->x,beamunitlib->y);
         ui->Plot1->replot();

         beamunitlib->set_data_show(1);

         ui->Plot2->graph(0)->setData(beamunitlib->x,beamunitlib->y);
         ui->Plot2->replot();

         sig_ask_status();

    }//end  if(cnt_cycle_view==Refresh_Cycle_View)

    if(beamunitlib->data_mng->Fl_Start)  SigReadyRecv(beamunitlib->ptr_data_recv, beamunitlib->data_mng->N_RCV*4);

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


void MainWindow::on_lineEdit_6_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_6->text().toDouble();
    Xm_2 = ui->lineEdit_7->text().toDouble();
    ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot1->replot();

    if(ui->checkBox_sync->isChecked()){
        ui->Plot2->xAxis->setRange(Xm_2, Xm_1);
        ui->Plot2->replot();
        ui->lineEdit_10->setText(QString::number(Xm_1));
        ui->lineEdit_11->setText(QString::number(Xm_2));
    }
}

void MainWindow::on_lineEdit_7_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_6->text().toDouble();
    Xm_2 = ui->lineEdit_7->text().toDouble();
    ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot1->replot();

    if(ui->checkBox_sync->isChecked()){
        ui->Plot2->xAxis->setRange(Xm_2, Xm_1);
        ui->Plot2->replot();
        ui->lineEdit_10->setText(QString::number(Xm_1));
        ui->lineEdit_11->setText(QString::number(Xm_2));
    }
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
    if(ui->checkBox_sync->isChecked()){
        ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
        ui->Plot1->replot();
        ui->lineEdit_6->setText(QString::number(Xm_1));
        ui->lineEdit_7->setText(QString::number(Xm_2));
    }
}

void MainWindow::on_lineEdit_11_returnPressed()
{
    double Xm_1;
    double Xm_2;
    Xm_1 = ui->lineEdit_10->text().toDouble();
    Xm_2 = ui->lineEdit_11->text().toDouble();
    ui->Plot2->xAxis->setRange(Xm_2, Xm_1);
    ui->Plot2->replot();
    if(ui->checkBox_sync->isChecked()){
        ui->Plot1->xAxis->setRange(Xm_2, Xm_1);
        ui->Plot1->replot();
        ui->lineEdit_6->setText(QString::number(Xm_1));
        ui->lineEdit_7->setText(QString::number(Xm_2));
    }
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


void MainWindow::slot_recv_status(QString str_stat){
   ui->statusBar->showMessage(str_stat);
}

void MainWindow::on_pushButton_clicked()
{
    if(beamunitlib->torsi_cntrl->Fl_simulator==true){
       beamunitlib->torsi_cntrl->Fl_simulator=false;
       ui->pushButton->setText("Run");
       sig_stop_simulator();
    }
    else{
       beamunitlib->torsi_cntrl->Fl_simulator=true;
       ui->pushButton->setText("Stop");
       sig_run_simulator(ui->lineEdit_2->text().toInt(), ui->lineEdit_5->text().toInt());
    }
}

void MainWindow::on_checkBox_wr_stateChanged(int arg1)
{

}

void MainWindow::slotPlotReplot2(){

    ui->lineEdit_3->setText(QString::number(ui->Plot1->yAxis->range().lower));
    ui->lineEdit_4->setText(QString::number(ui->Plot1->yAxis->range().upper));
    if(ui->checkBox_sync->isChecked()){
        ui->Plot2->yAxis->setRange(ui->Plot1->yAxis->range());
        ui->Plot2->xAxis->setRange(ui->Plot1->xAxis->range());
        ui->Plot2->replot();
        ui->lineEdit_8->setText(QString::number(ui->Plot1->yAxis->range().lower));
        ui->lineEdit_9->setText(QString::number(ui->Plot1->yAxis->range().upper));
    }
}
void MainWindow::slotPlotReplot1(){

    ui->lineEdit_8->setText(QString::number(ui->Plot1->yAxis->range().lower));
    ui->lineEdit_9->setText(QString::number(ui->Plot1->yAxis->range().upper));
    if(ui->checkBox_sync->isChecked()){
        ui->Plot1->yAxis->setRange(ui->Plot2->yAxis->range());
        ui->Plot1->xAxis->setRange(ui->Plot2->xAxis->range());
        ui->Plot1->replot();
        ui->lineEdit_3->setText(QString::number(ui->Plot2->yAxis->range().lower));
        ui->lineEdit_4->setText(QString::number(ui->Plot2->yAxis->range().upper));
    }
}

void MainWindow::on_lineEdit_2_editingFinished()
{
    sig_set_simulator(ui->lineEdit_2->text().toInt(),ui->lineEdit_5->text().toInt());
}
