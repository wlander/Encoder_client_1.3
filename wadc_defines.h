#ifndef WADC_DEFINES_H
#define WADC_DEFINES_H

#include <QVector>

//#define DEBUG_MODE

#define N_RCV_MAX 32768
#define N_RCV_DEF 2048
#define N_FLAG 16
#define BOTTOM_AXIS 256
#define ADC_CLK 14
#define NUM_BUF_TAH 32
#define POROG_BAD_TAH   15
#define NUM_CH  3

const int COM_BaudRate = 230400;

typedef volatile struct
{
    char Mode_Op;
    bool Fl_Proc;
    bool Fl_Connect; 	//+
    bool Fl_Start;		//+
    bool Fl_SD_Ready;
    bool Reg_On;	//+
    bool Mode_UDP;      //else MODE = COM_Port
    bool fl_rcv_data_en;
    bool fl_rcv_control_en;

    int N_RCV; //+
    int cnt_recv; //+
    int cnt_obr;	//+
    int cnt_byte_rec;
    int cnt_cdg_recv;
    int cnt_block_recv; //+
    int  Num_Ch;	//+
    int cnt_reg_recv; //+

    double  mx_ch[NUM_CH]; //+
    double  min_ch[NUM_CH]; //+

//-------------

    bool fl_write_to_file; //+
    int cnt_data_recv; //+

    int Refresh_Cycle_View;
    int cnt_cycle_view;
    int num_block_view = N_RCV_DEF;

}data_managing;

typedef volatile struct
{

    bool Tahometr_On;
    bool Fl_Porog;
    double PorogTah;
    int NumChTah;
    double Fmean;
    int CntObTah;
    int ObMinTah_Aver;
    double TimeObTah;
    double Sum_Tah;
    double CntAverTah;
    double TimeAverTah;
    int SumTimeAverTah;
    int cnt_mean;
    int Detect_Tah;
    int NumCh_Aver;
    bool Fl_simulator;

}torsi_control;


typedef volatile struct
{

    double  k_norm[NUM_CH]; //+

    double Fd; //+
    double Fd_Inv; //+

    double Pgd_sredn;
    double Pval_sredn;
    int cnt_p_sredn;

    bool Fl_0_Before;
    double cnt_time_0_before;
    double Sum0Before;
    double Time0Before;

    bool Fl_0_After;
    double cnt_time_0_after;
    double Sum0After;
    double Time0After;

    //Calc
    double M_val;
    double Pval;
    double Pgd;

    //Define
    double Kusil;
    double Msopr;
    double KPD_Red;
    double Per_Otnosh;

    bool  Fl_Tou;
    bool  Fl_mean_set;
}luch;


//for Encoder mode

typedef struct
{
    char code_start_cdg;
    char type_p;
    char switch_p;
    char mode_p;
    char filter_ch1;
    char filter_ch2;

}encoder_params;



#endif // WADC_DEFINES_H
