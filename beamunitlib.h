#ifndef BEAMUNITLIB_H
#define BEAMUNITLIB_H

#include <QObject>
#include "wadc_defines.h"

class BeamUnitLib : public QObject
{
    Q_OBJECT
public:
    explicit BeamUnitLib(QObject *parent = nullptr);
    ~BeamUnitLib();
signals:

public slots:
    int Handler_unit(char*, unsigned int);
    int write_to_file_reg_data(int p);
    int set_header(char*);
public:

    QByteArray reg_buf;
    data_managing* data_mng;
    torsi_control* torsi_cntrl;
    cdg_control_sd* cntrl_sd_ptr;
    QString filereg;
    unsigned num_data_recv;

    QString strr;
    QString Fd_str;
    QString Volts_inv_str;

private:

};

#endif // BEAMUNITLIB_H
