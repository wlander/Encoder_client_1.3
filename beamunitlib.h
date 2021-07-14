#ifndef BEAMUNITLIB_H
#define BEAMUNITLIB_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include "wadc_defines.h"

class Codogramms{
public:

    Codogramms(): cur_set(0){}

    void set_set(int n){
        if(n<max_cdg-1) cur_set = n;
        else qDebug()<<"error set set out of range!"<<endl;
    }

    const QString get_set(){
        return set[cur_set];
    }
    const QString get_stop(){
        return set[stop];
    }
private:

    enum cdg{
        usb_start,
        flash_start,
        flash_read,
        stop,
        max_cdg
    };

    const QString set[max_cdg] = {"$SET,1,1,3,3;\r\n", "$SET,1,3,3,3;\r\n", "$SET,1,4,3,3;\r\n", "$SET,2,1,3,3;\r\n"};

    int cur_set;

};


class BeamUnitLib : public QObject
{
    Q_OBJECT
public:
    explicit BeamUnitLib(QObject *parent = nullptr);
    ~BeamUnitLib();

    int get_n_rcv(){return data_mng->N_RCV;}
    bool get_fl_connect(){return data_mng->Fl_Connect;}
    bool get_fl_start(){return data_mng->Fl_Start;}
    int get_cycle_view(){return data_mng->cnt_cycle_view;}
    int get_refresh_view(){return data_mng->Refresh_Cycle_View;}
    int get_tah_ob(){return torsi_cntrl->CntObTah;}
    int get_ob_aver(){return torsi_cntrl->ObMinTah_Aver;}
    double get_thr_err(){return torsi_cntrl->PorogTah;}
    double get_freq(){return torsi_cntrl->Fmean;}
    int get_cnt_recv(){return data_mng->cnt_recv;}
    double get_max_ch(int ch){
        if(ch<NUM_CH) return data_mng->mx_ch[ch];
        else return 0;
    }
    double get_min_ch(int ch){
        if(ch<NUM_CH) return data_mng->min_ch[ch];
        else return 0;
    }

    bool get_fl_simulator(){return torsi_cntrl->Fl_simulator;}

signals:

    void sig_proc_comlplete();
    void SigReadyRecv(char*, int);

public slots:

    void Handler_unit();
    void set_data_show(int);

    //setters slots
    void set_buf_size(int n){data_mng->N_RCV = n;}
    void set_data_connect(bool fl){data_mng->Fl_Connect = fl;}
    void set_data_start(bool fl){data_mng->Fl_Start = fl;}
    void set_cycle_view(int n){data_mng->Refresh_Cycle_View = n;}
    void set_thr_err(double p){torsi_cntrl->PorogTah = p;}
    void set_tah_on(bool fl){torsi_cntrl->Tahometr_On = fl;}
    void set_time_tah(double p){torsi_cntrl->TimeAverTah = p;}
    void set_simulator(bool fl){torsi_cntrl->Fl_simulator = fl;}
    void set_reinit();
    void set_proc_en(bool fl){data_mng->Fl_Proc = fl;}

public:

    QString strr;
    QString Fd_str;
    QString Volts_inv_str;

    double  obr_buf_f[NUM_CH][N_RCV_MAX];
    QVector<double> x, y;

    char* ptr_data_recv;

    Codogramms cdg_str;

private:

    data_managing* data_mng;
    torsi_control* torsi_cntrl;


    void init_all();

    float data_buf[N_RCV_MAX];
};


#endif // BEAMUNITLIB_H
