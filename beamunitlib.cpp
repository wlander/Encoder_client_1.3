#include "beamunitlib.h"
#include <qdebug.h>
#include <QString>
#include <QFileDialog>
#include "wadc_defines.h"


BeamUnitLib::BeamUnitLib(QObject *parent) : QObject(parent)
{
    cntrl_sd_ptr = new cdg_control_sd;
    data_mng = new data_managing;
    torsi_cntrl = new torsi_control;

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

    data_mng->Fd = 7000.0;
    data_mng->Fd_Inv = 1.0/data_mng->Fd;

    data_mng->k_norm[0] = 1.0;
    data_mng->k_norm[1] = 1.0;
    data_mng->k_norm[2] = 1.0;

    num_data_recv = ((SIZE_DATA_CDG*2)+16);

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
    torsi_cntrl->Pgd_sredn = 0;
    torsi_cntrl->Pval_sredn = 0;
    torsi_cntrl->cnt_p_sredn = 0;
    torsi_cntrl->NumCh_Aver = 0;
    torsi_cntrl->Fl_0_Before = false;
    torsi_cntrl->cnt_time_0_before = 0.0;
    torsi_cntrl->Sum0Before = 0.0;
    torsi_cntrl->Time0Before = 10.0;

    torsi_cntrl->Fl_0_After = false;
    torsi_cntrl->cnt_time_0_after = 0.0;
    torsi_cntrl->Sum0After = 0.0;
    torsi_cntrl->Time0After = 10.0;

    torsi_cntrl->M_val = 0;
    torsi_cntrl->Pval = 0;
    torsi_cntrl->Pgd = 0;

    torsi_cntrl->Kusil = 1.0;
    torsi_cntrl->Msopr = 1.0;
    torsi_cntrl->KPD_Red = 0.5;
    torsi_cntrl->Per_Otnosh = 1.0;


    qDebug("Hello from BeamUnitLib!");
}

BeamUnitLib::~BeamUnitLib()
{
    qDebug("By BeamUnitLib!");
    delete [] cntrl_sd_ptr;
    delete [] data_mng;
}

int BeamUnitLib::Handler_unit(char* data_buf, unsigned int num)
{
   unsigned int i,j;
   QString str;
   unsigned start_pos_data=0;
   unsigned kk = 0;
   double sample1;
   double Tau = 0;
   double Kus = 0;
   double buf_filter_out[N_RCV_MAX*2];
   float* ptr_enc_data;
   char* rcv_buf = data_buf;
   unsigned int num_data_obr = num;

    if(torsi_cntrl->Fl_Tou/*ui->checkBox_2->isChecked()==true*/){
        Tau = torsi_cntrl->Sum0Before;
        Kus = torsi_cntrl->Kusil;
    }else{
        Tau = 0;
        Kus = 1.0;
    }


    data_mng->cnt_block_recv = 0;
    start_pos_data=0;
    kk = 0;

    if(num_data_obr>N_RCV_MAX){
        //ui->label_19->setText("Переполнение буфера обработки"); !!!???
        num_data_obr = N_RCV_MAX;
    }

    //!!!!!!!!!! ENCODER RECV MODE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if(data_mng->Reg_On){
        reg_buf.append(data_buf, num);
        data_mng->cnt_reg_recv+=num;
    }

        ptr_enc_data = (float*)(rcv_buf);

        start_pos_data = 0;
        kk = 0;

        data_mng->obr_buf_f[1][0] = 0;
        //unpack samples
        for(i=start_pos_data;i<num_data_obr/4;i++){

            if(ptr_enc_data[i]>0){
                data_mng->obr_buf_f[0][kk] = (double)ptr_enc_data[i];
                kk++;
                data_mng->obr_buf_f[1][kk] = 0;
                data_mng->cnt_block_recv++;
            }
            else{
                data_mng->obr_buf_f[1][kk] = 1;
            }

        }//end unpack

    //!!!!!!!!!!!!!! SIGNAL PROCESSING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        data_mng->cnt_cycle_view++;
        data_mng->cnt_recv++;
        data_mng->cnt_data_recv+=data_mng->cnt_block_recv;

        torsi_cntrl->Tahometr_On=false; //!!!

        //Тахометр
        if((torsi_cntrl->Tahometr_On==true)&&(data_mng->cnt_block_recv>1)){

            torsi_cntrl->Detect_Tah++;

            for(i=0;i<data_mng->cnt_block_recv-1;i++){
                torsi_cntrl->win_mean_p[torsi_cntrl->cnt_mean] = data_mng->obr_buf_f[torsi_cntrl->NumChTah][i];
                torsi_cntrl->cnt_mean++;
                if(torsi_cntrl->cnt_mean==NUM_MEAN_P) torsi_cntrl->cnt_mean = 0;
                buf_filter_out[i] = 0;
                for(j=0;j<NUM_MEAN_P;j++) buf_filter_out[i] += torsi_cntrl->win_mean_p[j];
                data_mng->obr_buf_f[torsi_cntrl->NumChTah][i] = buf_filter_out[i]/(double)NUM_MEAN_P;
            }


            for(i=0;i<data_mng->cnt_block_recv-1;i++){

                  //Tahometer
                  sample1 = data_mng->obr_buf_f[torsi_cntrl->NumChTah][i]; //buf_filter_out[i];
                  if(sample1<0) sample1 = -sample1;

                  if(sample1>torsi_cntrl->PorogTah){

                      if((torsi_cntrl->Fl_Porog==false)&&(torsi_cntrl->TimeObTah>POROG_BAD_TAH)){
                          torsi_cntrl->Fl_Porog=true;
                          torsi_cntrl->CntObTah+=1;
                          torsi_cntrl->Sum_Tah += (float)torsi_cntrl->TimeObTah*data_mng->Fd_Inv;
                          torsi_cntrl->CntAverTah++;
                          torsi_cntrl->TimeObTah = 0;
                          torsi_cntrl->Detect_Tah = 0;
                      }

                  }
                  else{
                      torsi_cntrl->Fl_Porog=false;
                  }

                  torsi_cntrl->TimeObTah+=1;

                  //Aver_Vol
                  torsi_cntrl->Sum_Vol+=data_mng->obr_buf_f[torsi_cntrl->NumChVol][i];
                  torsi_cntrl->CntAverVol++;

             }

             if(((torsi_cntrl->Sum_Tah>=torsi_cntrl->TimeAverTah)&&(torsi_cntrl->TimeAverTah>0)) || (((torsi_cntrl->CntObTah%2)==0)&&(torsi_cntrl->TimeAverTah==0))){
                 //ObMinTah_Aver = 60.0/(Sum_Tah/(double)CntAverTah);

                 torsi_cntrl->ObMinTah_Aver = (60.0*(double)torsi_cntrl->CntAverTah)/torsi_cntrl->Sum_Tah;

                 //???? ui->label_tah->setText(QString::number(torsi_cntrl->ObMinTah_Aver)); +++

                 torsi_cntrl->CntAverTah = 0;
                 torsi_cntrl->Sum_Tah = 0.000001;
             }


             if(torsi_cntrl->Detect_Tah>10){
                //???? ui->label_tah->setText("--------"); +++
                torsi_cntrl->CntObTah = 0;
             }


             if((torsi_cntrl->CntAverVol*data_mng->Fd_Inv)>torsi_cntrl->TimeAverVol){
                 torsi_cntrl->Aver_Vol = torsi_cntrl->Sum_Vol/torsi_cntrl->CntAverVol++;
                 torsi_cntrl->Sum_Vol = 0;
                 torsi_cntrl->CntAverVol = 0.000001;
             }

             //Calc params
             torsi_cntrl->M_val = torsi_cntrl->Aver_Vol*torsi_cntrl->Msopr;
             torsi_cntrl->Pval = torsi_cntrl->M_val*0.000105*torsi_cntrl->ObMinTah_Aver;
             torsi_cntrl->Pgd = (torsi_cntrl->M_val*0.000105*torsi_cntrl->ObMinTah_Aver)/(torsi_cntrl->KPD_Red*torsi_cntrl->Per_Otnosh);


             torsi_cntrl->Pgd_sredn += torsi_cntrl->Pgd;
             torsi_cntrl->Pval_sredn += torsi_cntrl->Pval;
             torsi_cntrl->cnt_p_sredn++;

        }//end Tah

             //вычисление 0/До
        if((torsi_cntrl->Fl_0_Before) || (torsi_cntrl->Fl_0_After)){

             for(i=0;i<data_mng->cnt_block_recv;i++){
                     torsi_cntrl->Sum0Before+=data_mng->obr_buf_f[torsi_cntrl->NumCh_Aver][i];
                     torsi_cntrl->cnt_time_0_before+=1.0;
             }
                 if((torsi_cntrl->cnt_time_0_before*data_mng->Fd_Inv)>=torsi_cntrl->Time0Before){
                    torsi_cntrl->Sum0Before = torsi_cntrl->Sum0Before/torsi_cntrl->cnt_time_0_before;
                    if(torsi_cntrl->Fl_0_Before){
                         //str = "Средн До: " + QString::number(torsi_cntrl->Sum0Before);
                         //???? ui->label_13->setText(str); +++
                         torsi_cntrl->Fl_0_Before = 0;
                    }
                    else{
                        //str = "Средн После: " + QString::number(torsi_cntrl->Sum0Before);
                        //???? ui->label_9->setText(str); +++
                        torsi_cntrl->Fl_0_After = 0;
                    }
                    torsi_cntrl->cnt_time_0_before = 0.00001;
                    //???? ui->pushButton_2->setEnabled(true); +++
                    //???? ui->pushButton_7->setEnabled(true); +++

                    torsi_cntrl->Fl_mean_set = true;
                 }

        }

    return 0;
}

int BeamUnitLib::write_to_file_reg_data(int p){
    QFile file;
    QString file1;
    char* ptr_reg_data;
    unsigned start_pos_data=0;
    int i = 0;
    unsigned short sample = 0;
    double fsample = 0.0;
    unsigned int sample_u;
    char* ptr_sample_u = (char*)&sample_u;
    unsigned int kk=0;

    char* ptr_sample = (char*)&sample;

    if((reg_buf.size()>(DELETE_DATA*2))&&(filereg.length()>0)){

        ptr_reg_data = reg_buf.data();
        start_pos_data=0;
        kk = 0;

        //Looking for the START_ID_DATA

        for(i=(DELETE_DATA*2);i<reg_buf.size()/2;i++){

            ptr_sample_u[0] = ptr_reg_data[kk];
            ptr_sample_u[1] = ptr_reg_data[kk+1];
            ptr_sample_u[2] = ptr_reg_data[kk+2];
            ptr_sample_u[3] = ptr_reg_data[kk+3];

            if(sample_u==START_ID_DATA2){
                start_pos_data = kk+HEADER_LEN2;
                break;
            }
            kk++;
        }

        //записываем файл напряжений
        file1 = filereg+"_1";
        file.setFileName(file1);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)){

             QTextStream stream(&file);
             ptr_reg_data = reg_buf.data();

             kk = start_pos_data;

             while(kk<(reg_buf.size()-4)){

                ptr_sample_u[0] = ptr_reg_data[kk+0];
                ptr_sample_u[1] = ptr_reg_data[kk+1];
                ptr_sample_u[2] = ptr_reg_data[kk+2];
                ptr_sample_u[3] = ptr_reg_data[kk+3];

                if(sample_u!=START_ID_DATA2){
                    ptr_sample[0] = ptr_reg_data[kk];
                    ptr_sample[1] = ptr_reg_data[kk+1];
                    fsample = ((double)((sample&0x03FF)<<2))*data_mng->k_norm[0];
                    stream<<QString::number(fsample)<<"\n";
                    kk+=2;
                }
                else{
                    kk+=4;
                }

             }

             stream<<QString::number(0);

        }
        file.close();

        //записываем файл импульсов
        file1 = filereg+"_2";
        file.setFileName(file1);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)){

             QTextStream stream(&file);
             ptr_reg_data = reg_buf.data();


             kk = start_pos_data;

             while(kk<(reg_buf.size()-4)){

                ptr_sample_u[0] = ptr_reg_data[kk+0];
                ptr_sample_u[1] = ptr_reg_data[kk+1];
                ptr_sample_u[2] = ptr_reg_data[kk+2];
                ptr_sample_u[3] = ptr_reg_data[kk+3];

                if(sample_u!=START_ID_DATA2){
                    sample = 0;
                    ptr_sample[0] = ptr_reg_data[kk+1];
                    ptr_sample[0] &= 0xFC;
                    fsample = ((double)(sample<<3))*data_mng->k_norm[0];
                    stream<<QString::number(fsample)<<"\n";
                    kk+=2;
                }
                else{
                    kk+=4;
                }

             }

             stream<<QString::number(0);

        }
        file.close();
    }

    return 0;

}


int BeamUnitLib::set_header(char* data){
    char* struct_ptr = (char*)this->cntrl_sd_ptr;

    for(int i=0;i<sizeof(cdg_control_sd);i++) struct_ptr[i] = data[i];

    return 0;
}
