#ifndef BEAMUNITLIB_H
#define BEAMUNITLIB_H

#include <QObject>
#include <QVector>
#include "wadc_defines.h"

class BeamUnitLib : public QObject
{
    Q_OBJECT
public:
    explicit BeamUnitLib(QObject *parent = nullptr);
    ~BeamUnitLib();
signals:

public slots:
    int Handler_unit();
    int set_header(char*);
    void set_data_show(int);

    //setters slots
    void set_buf_size(int);
    void set_data_connect(bool);
    void set_data_start(bool);
    void set_reinit();
    void set_cycle_view(int);
    void set_thr_err(double);

public:

    data_managing* data_mng;
    torsi_control* torsi_cntrl;
    cdg_control_sd* cntrl_sd_ptr;
    unsigned num_data_recv;
    QString strr;
    QString Fd_str;
    QString Volts_inv_str;

    double  obr_buf_f[3][N_RCV_MAX];
    QVector<double> x, y;

    char* ptr_data_recv;

private:
    void init_all();

    float data_buf[N_RCV_MAX];
};

#endif // BEAMUNITLIB_H
