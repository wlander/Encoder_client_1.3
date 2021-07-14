#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QString>
#include <QDesktopWidget>
#include <QScreen>
#include <QFileDialog>
#include <QMetaEnum>
#include <unistd.h>
#include <errno.h>
#include <QThread>
#include <QDateTime>
#include "fwriter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);  

    ui->label_tah->setStyleSheet("QLabel { background-color : white; }");
    ui->Plot1->setStyleSheet("QLabel { background-color : white; }");
    ui->Plot2->setStyleSheet("QLabel { background-color : white; }");
 //   this->setStyleSheet("QLabel { background-color : gray; }");

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

    ui->checkBox_proc->setChecked(true);
    ui->checkBox_wr->setChecked(true);

    ui->groupBox_2->setVisible(false);
    ui->checkBox_2->setVisible(false);
    ui->pushButton_2->setVisible(false);
    ui->pushButton_7->setVisible(false);
    ui->label_13->setVisible(false);
    ui->label_9->setVisible(false);
    ui->groupBox_3->setVisible(false);

//-------------------- Custom manage and processing class -----------------------------------------------------
    beamunitlib = new BeamUnitLib();
    QThread *thread_proc = new QThread;
    beamunitlib->moveToThread(thread_proc);

    connect(this, SIGNAL(set_tah_on(bool)), beamunitlib, SLOT(set_tah_on(bool)));
    connect(this, SIGNAL(set_buf_size(int)), beamunitlib, SLOT(set_buf_size(int)));
    connect(this, SIGNAL(set_data_connect(bool)), beamunitlib, SLOT(set_data_connect(bool)));
    connect(this, SIGNAL(set_data_start(bool)), beamunitlib, SLOT(set_data_start(bool)));
    connect(this, SIGNAL(set_reinit()), beamunitlib, SLOT(set_reinit()));
    connect(this, SIGNAL(set_cycle_view(int)), beamunitlib, SLOT(set_cycle_view(int)));
    connect(this, SIGNAL(set_thr_err(double)), beamunitlib, SLOT(set_thr_err(double)));
    connect(this, SIGNAL(set_time_tah(double)), beamunitlib, SLOT(set_time_tah(double)));
    connect(this, SIGNAL(set_simulator(bool)), beamunitlib, SLOT(set_simulator(bool)));
    connect(this, SIGNAL(set_proc_en(bool)), beamunitlib, SLOT(set_proc_en(bool)));

//----------- настройка QCustomPlot -----------------------------------------------
    ui->Plot1->addGraph();
    ui->Plot2->addGraph();

    ui->Plot1->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    ui->Plot2->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->Plot1->xAxis->rescale(true);
    ui->Plot2->xAxis->rescale(true);

    ui->Plot1->graph(0)->setData(beamunitlib->x,beamunitlib->y);
    ui->Plot2->graph(0)->setData(beamunitlib->x,beamunitlib->y);

    ui->Plot1->xAxis->setRange(0, beamunitlib->get_n_rcv());
    ui->Plot1->yAxis->setRange(-2.0, 2.0);
    ui->Plot2->xAxis->setRange(0, beamunitlib->get_n_rcv());
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

//------------- запуск и настройка потока с классом COM порта ---------------------

    QThread *thread_port = new QThread;//Создаем поток для порта платы
    PortC = new Port();//Создаем обьект по классу

    PortC->moveToThread(thread_port);//помешаем класс  в поток
    PortC->CPort.moveToThread(thread_port);//Помещаем сам порт в поток
    connect(PortC, SIGNAL(sigError_(QString)), this, SLOT(slotErrPort(QString)));//Лог ошибок
    connect(thread_port, SIGNAL(started()), PortC, SLOT(slotProcessPort()));//Переназначения метода run
    connect(PortC, SIGNAL(sigFinishedPort()), thread_port, SLOT(quit()));//Переназначение метода выход
    connect(thread_port, SIGNAL(finished()), PortC, SLOT(deleteLater()));//Удалить к чертям поток
    connect(PortC, SIGNAL(sigFinishedPort()), thread_port, SLOT(deleteLater()));//Удалить к чертям поток
    connect(this,SIGNAL(savesettings(QString,int,int,int,int,int)),PortC,SLOT(slotWriteSettingsPort(QString,int,int,int,int,int)));//Слот - ввод настроек!
    connect(this,SIGNAL(Signal_SetRunReg(bool)),PortC,SLOT(slotSetRecordEn(bool)));
    connect(this, SIGNAL(Connect_Port()),PortC,SLOT(slotConnectPort()));//по нажатию кнопки подключить порт
    connect(this, SIGNAL(Disconnect_Port()),PortC,SLOT(slotDisconnectPort()));//по нажатию кнопки отключить порт
    connect(this,SIGNAL(writeData(QByteArray)),PortC,SLOT(slotWriteToPort(QByteArray)));//отправка в порт данных
    connect(this, SIGNAL(sig_ask_status()), PortC, SLOT(slotGetStatus()));
    connect(PortC, SIGNAL(sigStatus(QString)), this, SLOT(slot_recv_status(QString)));
    connect(this, SIGNAL(sig_stop_simulator()),PortC,SLOT(slotSimulatorStop()));//
    connect(this, SIGNAL(sig_run_simulator(int, int)),PortC,SLOT(slotSimulatorStart(int, int)));//
    connect(this, SIGNAL(sig_set_simulator(int,int)), PortC, SLOT(slotInitSimulator(int,int)));
    connect(this, SIGNAL(sigPortClear()), PortC, SLOT(slotClearPort()));
    connect(PortC, SIGNAL(sigPortOpened()), this, SLOT(slotPortOpened()));

    connect(PortC, SIGNAL(sigSendPortData()), beamunitlib, SLOT(Handler_unit()));
    connect(beamunitlib, SIGNAL(sig_proc_comlplete()), this, SLOT(ProcStat()));
    connect(beamunitlib,SIGNAL(SigReadyRecv(char*, int)),PortC,SLOT(slotSetReadyRead(char*, int)));

    fwrtr = new fwriter();
    QThread *thread_filewr = new QThread;//
    fwrtr->moveToThread(thread_filewr);

    connect(PortC, SIGNAL(sigSendRecordData(QByteArray)), fwrtr, SLOT(proc(QByteArray)));
    connect(fwrtr, SIGNAL(fwriter_ready()), PortC, SLOT(slotGetRecordData()));

    thread_proc->start();
    thread_filewr->start();
    thread_port->start();

    emit set_tah_on(true);
    emit set_time_tah(ui->spinBox_2->text().toDouble());
    emit set_thr_err(ui->lineEdit->text().toDouble());
    PortC->slotSetBufSize(beamunitlib->get_n_rcv()*4);

//---------------------------------------------------------------------------------

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Button_Connect_clicked()
{
    int i;

    if(beamunitlib->get_fl_connect()==false){

        savesettings(ui->PortNameEdit->text(),COM_BaudRate,8,0,1,0);
        Connect_Port();
        for(i=0;i<1000000;i++){i=i;}

    }else{

        emit set_data_connect(false);
        ui->Button_Connect->setText("Подключить");
        ui->Button_Start_Stop->setEnabled(false);
        Disconnect_Port();
        ui->label_19->setText("Отключено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: black;");
        ui->PortNameEdit->setEnabled(true);
        for(i=0;i<1000000;i++){i=i;}

    }

}

void MainWindow::slotPortOpened(){
    emit set_data_connect(true);
    ui->Button_Connect->setText("Отключить");
    ui->Button_Start_Stop->setEnabled(true);
    ui->label_19->setText("Остановлено");
    ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
    ui->PortNameEdit->setEnabled(false);
}

void MainWindow::slotErrPort(QString s){
QMessageBox msgBox;

    msgBox.setText("Ошибка подключения!");
    msgBox.exec();
}

void MainWindow::on_Button_Start_Stop_clicked()
{
  unsigned int i;
  QByteArray data_cdg;
  QString str = "";
  QMessageBox msgBox;
  QDate cdate = QDate::currentDate();
  QTime ctime = QTime::currentTime();

    if(beamunitlib->get_fl_start()==false){

        if((ui->checkBox_wr->isChecked())&&(!ui->radioBtn_flash_wr->isChecked())){
            str = QFileDialog::getSaveFileName(0, "Файл для записи", "tors_"+cdate.toString("dd-MM-yy")+"__"+ctime.toString("hh-mm")+".dat");
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

        emit set_reinit();

        ui->Button_Start_Stop->setText("Стоп");

        ui->Button_Connect->setEnabled(false);

        if(ui->checkBox_wr->isChecked()){
            ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgreen; }");
        }

        ui->checkBox_wr->setEnabled(false);

//it's requred!!
        data_cdg.append(beamunitlib->cdg_str.get_set());
        writeData(data_cdg);
        data_cdg.clear();

//        ui->listWidget->addItem("Прием данных с АЦП запущен");

    }else{

        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
        set_data_start(false);
        ui->Button_Start_Stop->setText("Старт");
        ui->Button_Start_Stop->setStyleSheet("QPushButton { background-color : lightgray; }");
        ui->Button_Connect->setEnabled(true);

//it's requred!!
        data_cdg.append(beamunitlib->cdg_str.get_stop());
        writeData(data_cdg);
        data_cdg.clear();

        for(i=0;i<100000;i++){i=i;}

        ui->checkBox_wr->setEnabled(true);

//        ui->listWidget->addItem("Прием данных с АЦП остановлен");

    }
}


void MainWindow::ProcStat()
{
    size_t i;
    QString str;


    if(beamunitlib->get_fl_start()){
        ui->label_19->setText("Исправно");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: green;");
    }
    else{
        ui->label_19->setText("Остановлено");
        ui->label_19->setStyleSheet("font-weight: bold; font-size: 10pt; color: blue;");
    }


    if(beamunitlib->get_tah_ob()==0) ui->label_tah->setText("--------");
    else   ui->label_tah->setText(QString::number(beamunitlib->get_ob_aver()));

    str = "Тах, об: " + QString::number(beamunitlib->get_tah_ob());
    str += (" / Сред F, КГц: " + QString::number(1000.0/beamunitlib->get_freq(),'f', 3));
    ui->groupBox_7->setTitle(str);

    //view_ch_1,2
    beamunitlib->set_data_show(0);

    ui->Plot1->graph(0)->setData(beamunitlib->x,beamunitlib->y);
    ui->Plot1->replot();

    beamunitlib->set_data_show(1);

    ui->Plot2->graph(0)->setData(beamunitlib->x,beamunitlib->y);
    ui->Plot2->replot();

    sig_ask_status();

}


void MainWindow::on_pushButton_7_clicked()
{
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
}

void MainWindow::on_lineEdit_returnPressed()
{
    set_thr_err(ui->lineEdit->text().toDouble());
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

    if(beamunitlib->get_cnt_recv()>0){

        Ym_1 = beamunitlib->get_max_ch(0)*1.3;
        Ym_2 = beamunitlib->get_min_ch(0)*0.7;
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

    if(beamunitlib->get_cnt_recv()>0){

        Ym_1 = beamunitlib->get_max_ch(1)*1.3;
        Ym_2 = beamunitlib->get_min_ch(1)*0.7;
        ui->Plot2->yAxis->setRange(Ym_2, Ym_1);
        ui->Plot2->replot();
        ui->lineEdit_8->setText(QString::number(Ym_1));
        ui->lineEdit_9->setText(QString::number(Ym_2));

    }
}


void MainWindow::on_spinBox_2_valueChanged(const QString &arg1)
{
    set_time_tah(ui->spinBox_2->text().toDouble());
}

void MainWindow::on_lineEdit_editingFinished()
{
   set_thr_err(ui->lineEdit->text().toDouble());
}

void MainWindow::slot_recv_status(QString str_stat){
   ui->statusBar->showMessage(str_stat);
}

void MainWindow::on_pushButton_clicked()
{
    if(beamunitlib->get_fl_simulator()==true){

       set_simulator(false);

       ui->pushButton->setText("Run");
       sig_stop_simulator();
    }
    else{
       set_simulator(true);
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



void MainWindow::on_pushButton_2_clicked(){
//    beamunitlib->torsi_cntrl->Fl_0_After = 1;
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
}

void MainWindow::on_lineEdit_14_returnPressed(){
//    beamunitlib->torsi_cntrl->Kusil = ui->lineEdit_14->text().toDouble();
}
void MainWindow::on_lineEdit_15_returnPressed(){
//    beamunitlib->torsi_cntrl->Msopr = ui->lineEdit_15->text().toDouble();
}
void MainWindow::on_lineEdit_12_returnPressed(){
//   beamunitlib->torsi_cntrl->KPD_Red = ui->lineEdit_12->text().toDouble()/100.0;
}
void MainWindow::on_lineEdit_13_returnPressed(){
//    beamunitlib->torsi_cntrl->Per_Otnosh = ui->lineEdit_13->text().toDouble()/100.0;
}
void MainWindow::on_lineEdit_13_editingFinished(){
//    beamunitlib->torsi_cntrl->Per_Otnosh = ui->lineEdit_13->text().toDouble()/100.0;
}
void MainWindow::on_lineEdit_12_editingFinished(){
//    beamunitlib->torsi_cntrl->KPD_Red = ui->lineEdit_12->text().toDouble()/100.0;
}
void MainWindow::on_lineEdit_15_editingFinished(){
//   beamunitlib->torsi_cntrl->Msopr = ui->lineEdit_15->text().toDouble();
}
void MainWindow::on_lineEdit_14_editingFinished(){
//  beamunitlib->torsi_cntrl->Kusil = ui->lineEdit_14->text().toDouble();
}
void MainWindow::on_checkBox_2_stateChanged(int arg1){

}


void MainWindow::closeEvent(QCloseEvent *event)
{
/*
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
*/
}


void MainWindow::on_checkBox_proc_stateChanged(int arg1){
    emit set_proc_en(ui->checkBox_proc->isChecked());
}

void MainWindow::on_radioBtn_usb_clicked()
{
    beamunitlib->cdg_str.set_set(0);
}

void MainWindow::on_radioBtn_flash_wr_clicked()
{
    beamunitlib->cdg_str.set_set(1);
}

void MainWindow::on_radioBtn_flash_rd_clicked()
{
    beamunitlib->cdg_str.set_set(2);
}
