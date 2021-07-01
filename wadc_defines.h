#ifndef WADC_DEFINES_H
#define WADC_DEFINES_H

#include <QVector>

//#define DEBUG_MODE

#define N_RCV_MAX 32768
#define N_RCV_DEF 2048
#define N_FLAG 16
#define POS_CH1 2121
#define POS_CH2 2135
#define POS_CH1_SM 2041
#define POS_CH2_SM 2039
#define POS_VIEW_CH1 5000
#define BOTTOM_AXIS 256
#define ADC_CLK 14
#define NUM_BUF_TAH 32
#define START_ID_DATA  0xFEFE
#define HEADER_LEN  2
#define MAX_ID_BUF  128
#define OVERFLOW_DATA  0xFFFF
#define DELETE_DATA  6000

#define HEADER_LEN2  4
#define START_ID_DATA2  0xFF0000FF

#define HEADER_TYPE2

#define POROG_BAD_TAH   15

#define NUM_MEAN_P  3

#define SIZE_DATA_CDG 	30

const char Conf_Byte_Start = 0x31;
const char Conf_Byte_Stop = 0x30;
const char Conf_Byte_Test1 = 2;

const char Conf_Byte_SD_On = 6;
const char Conf_Byte_SD_Off = 7;
const char Conf_Byte_SD_WR_Start = 3;
const char Conf_Byte_SD_RD_Start = 4;
const char Conf_Byte_SD_Stop = 5;
const char Conf_Byte_SD_WR_Reset = 8;
const char Conf_Byte_SD_RD_Reset = 9;
const char Conf_Byte_Window_Mode_Start = 26;
const char Conf_Byte_Window_Mode_Stop = 27;
const char Conf_Byte_Pack_Mode_En = 28;
const char Conf_Byte_Pack_Mode_Dis = 29;
const char Conf_Byte_Pack_Mode_Ch1 = 30;
const char Conf_Byte_Pack_Mode_Ch2 = 31;
const char Conf_Byte_Recv_ad8555_Par = 35;
const char Conf_Byte_On_ad8555 = 36;
const char Conf_Byte_Off_ad8555 = 37;
const char Conf_Byte_Enc_Porog = 38;
const char Conf_Byte_Enc_On = 39;
const char Conf_Byte_Enc_Off = 40;
const char Conf_Byte_Enc_View_On = 41;
const char Conf_Byte_Enc_View_Off = 42;

const int COM_BaudRate = 230400;

typedef volatile struct
{
    char Mode_Op;
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

    double Fd; //+
    double Fd_Inv; //+

    double  mx_ch[3]; //+
    double  min_ch[3]; //+
    double  k_norm[3]; //+

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
    int NumChVol;
    int CntObTah;
    int ObMinTah_Aver;
    double TimeObTah;
    double Sum_Tah;
    double CntAverTah;
    double TimeAverTah;
    int SumTimeAverTah;
    double win_mean_p[NUM_MEAN_P];
    int cnt_mean;
    double TimeAverVol;
    double Sum_Vol;
    double Aver_Vol;
    int CntAverVol;
    int Detect_Tah;
    double Pgd_sredn;
    double Pval_sredn;
    int cnt_p_sredn;

    int NumCh_Aver;

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

    bool Fl_simulator;

}torsi_control;

typedef volatile struct
{
    uint32_t cnt_status_write;
    uint32_t cnt_file_sd;
    uint32_t cnt_status_write_all;
    uint8_t stat[4];

    uint16_t data[SIZE_DATA_CDG];

}cdg_control_sd;

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

#define START_ID_DATA_ENC  0x3FFFFFFF
#define SPEED_MARK_ENC  0xFFFFFFFF

#endif // WADC_DEFINES_H
