#include "beamunitlib.h"
#include <qdebug.h>
#include <QString>
#include <QFileDialog>
#include "wadc_defines.h"


BeamUnitLib::BeamUnitLib(QObject *parent) : QObject(parent)
{

    data_mng = new data_managing;
    torsi_cntrl = new torsi_control;
    //data_buf = new float[N_RCV_MAX];
    ptr_data_recv = (char*)data_buf;
    init_all();

    qDebug("Hello from BeamUnitLib!");
}

void BeamUnitLib::init_all(){

    data_mng->Fl_Connect = false;
    data_mng->Fl_Start = false;
    data_mng->Fl_SD_Ready = false;
    data_mng->Reg_On = false;
    data_mng->Mode_UDP = false;      //else MODE = COM_Port
    data_mng->fl_rcv_data_en = false;
    data_mng->fl_rcv_control_en = false;

    data_mng->N_RCV = N_RCV_DEF;
    data_mng->cnt_recv = 0;
    data_mng->cnt_obr = 0;
    data_mng->cnt_byte_rec = 0;
    data_mng->cnt_cdg_recv = 0;
    data_mng->cnt_block_recv = 0;
    data_mng->Num_Ch = 3;
    data_mng->cnt_reg_recv = 0;

    data_mng->Refresh_Cycle_View = 2;
    data_mng->cnt_cycle_view = 0;
    data_mng->num_block_view = N_RCV_DEF;

    torsi_cntrl->Tahometr_On = true;
    torsi_cntrl->Fl_Porog = true;
    torsi_cntrl->PorogTah = 0.4;
    torsi_cntrl->NumChTah = 1;
    torsi_cntrl->NumChVol = 0;
    torsi_cntrl->CntObTah = 0;
    torsi_cntrl->ObMinTah_Aver = 0;
    torsi_cntrl->TimeObTah = 0;
    torsi_cntrl->Sum_Tah = 0;
    torsi_cntrl->CntAverTah = 0;
    torsi_cntrl->TimeAverTah = 1;
    torsi_cntrl->SumTimeAverTah = 0;
    torsi_cntrl->cnt_mean = 0;
    torsi_cntrl->TimeAverVol = 1;
    torsi_cntrl->Sum_Vol = 0;
    torsi_cntrl->Aver_Vol = 1;
    torsi_cntrl->CntAverVol = 0;
    torsi_cntrl->Detect_Tah = 0;
    torsi_cntrl->NumCh_Aver = 0;

    x.resize(N_RCV_MAX*2);
    y.resize(N_RCV_MAX*2);

    for(int i = 0; i < data_mng->N_RCV; i++){
       x[i] = i;
       y[i] = -4.0+static_cast<double>(i)*(8.0/static_cast<double>(data_mng->N_RCV));
    }

}

BeamUnitLib::~BeamUnitLib()
{
    qDebug("By BeamUnitLib!");
    delete [] data_mng;
    delete [] torsi_cntrl;
    //delete data_buf;
}

void BeamUnitLib::Handler_unit()
{
   unsigned int i;
   QString str;
   unsigned start_pos_data=0;
   unsigned kk = 0;
   double summ_t = 0;
   double meann = 0;
   double diff = 0;

    data_mng->cnt_block_recv = 0;
    start_pos_data=0;
    kk = 0;

    //!!!!!!!!!! ENCODER RECV MODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        start_pos_data = 0;
        kk = 0;

        obr_buf_f[1][0] = 0;

        //unpack samples
        for(i=start_pos_data;i<data_mng->N_RCV;i++){

            if(data_buf[i]>0){
                obr_buf_f[0][kk] = static_cast<double>(data_buf[i]);
                summ_t+=obr_buf_f[0][kk];
                kk++;
                obr_buf_f[1][kk] = 0;
                data_mng->cnt_block_recv++;
            }
            else{
                obr_buf_f[1][kk] = 1;
            }

        }//end unpack


    //!!!!!!!!!!!!!! SIGNAL PROCESSING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        data_mng->cnt_cycle_view++;
        data_mng->cnt_recv++;
        data_mng->cnt_data_recv+=data_mng->cnt_block_recv;

        torsi_cntrl->Tahometr_On=true; //!!!

        //Тахометр
        if((torsi_cntrl->Tahometr_On==true)&&(data_mng->cnt_block_recv>1)){

            torsi_cntrl->Detect_Tah++;

            meann = summ_t/data_mng->cnt_block_recv;

            kk = 0;

            if(data_mng->cnt_block_recv>1){
                for(i=0;i<data_mng->cnt_block_recv-1;i++){
                    diff = meann-obr_buf_f[0][i];
                    if(diff<0.0) diff = -diff;
                    if(diff<torsi_cntrl->PorogTah){
                        torsi_cntrl->TimeObTah+=obr_buf_f[0][i];
                        kk++;
                    }
                    if(obr_buf_f[1][i]>0){
                        torsi_cntrl->Fl_Porog=true;
                        torsi_cntrl->CntObTah+=1;
                        torsi_cntrl->Sum_Tah += (torsi_cntrl->TimeObTah/1000000.0);
                        torsi_cntrl->CntAverTah+=1.0;
                        torsi_cntrl->TimeObTah = 0;
                        torsi_cntrl->Detect_Tah = 0;
                    }
                    else{
                      torsi_cntrl->Fl_Porog=false;
                    }
                }

            }

            if(((torsi_cntrl->Sum_Tah>=torsi_cntrl->TimeAverTah)&&(torsi_cntrl->TimeAverTah>0)) || (((torsi_cntrl->CntObTah%2)==0)&&(torsi_cntrl->TimeAverTah==0))){
                 torsi_cntrl->ObMinTah_Aver = static_cast<int>((60.0*torsi_cntrl->CntAverTah)/torsi_cntrl->Sum_Tah);
                 torsi_cntrl->CntAverTah = 0.0;
                 torsi_cntrl->Sum_Tah = 0.000001;
            }

            if(torsi_cntrl->Detect_Tah>10){
                torsi_cntrl->CntObTah = 0;
            }

        }//end Tah

    if((data_mng->cnt_cycle_view>=data_mng->Refresh_Cycle_View) && (data_mng->Fl_Proc)){
        data_mng->cnt_cycle_view = 0;
        emit sig_proc_comlplete();
    }

    emit SigReadyRecv(ptr_data_recv, get_n_rcv()*4);

}

void BeamUnitLib::set_data_show(int num_ch){
    double mx,mn;

    if(num_ch<data_mng->Num_Ch){

        mx = obr_buf_f[num_ch][0];
        mn = obr_buf_f[num_ch][0];
        for(int i = 0; i < data_mng->num_block_view; i++){
           x[i] = i;
           y[i] = obr_buf_f[num_ch][i];
           if(mx<y[i]) mx = y[i];
           if(mn>y[i]) mn = y[i];
        }

        data_mng->mx_ch[num_ch]=mx;
        data_mng->min_ch[num_ch]=mn;
    }
    else{
        qDebug()<<"Out of Range Num_Ch"<<endl;
    }

}

//setters

void BeamUnitLib::set_reinit(){
    data_mng->cnt_reg_recv = 0;
    data_mng->cnt_recv = 0;
    data_mng->cnt_data_recv = 0;
    data_mng->Fl_Start = true;
    torsi_cntrl->CntObTah = 0;
    emit SigReadyRecv(ptr_data_recv, get_n_rcv()*4);
}

