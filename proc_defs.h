#ifndef PROC_DEFS_H
#define PROC_DEFS_H


volatile bool Tahometr_On = true; //+
volatile bool Fl_Porog = true;
volatile double PorogTah = 0.4;
volatile unsigned NumChTah = 1;
volatile unsigned NumChVol = 0;
volatile unsigned CntObTah = 0;
volatile unsigned ObMinTah_Aver = 0;
volatile unsigned TimeObTah = 0;
volatile double Sum_Tah = 0;
volatile unsigned CntAverTah = 0;
volatile double TimeAverTah = 1;
volatile unsigned SumTimeAverTah = 0;
volatile double win_mean_p[NUM_MEAN_P] = {0};
volatile unsigned short cnt_mean = 0;
volatile double TimeAverVol = 1;
volatile double Sum_Vol = 0;
volatile double Aver_Vol = 1;
volatile unsigned CntAverVol = 0;
volatile unsigned Detect_Tah = 0;
double Pgd_sredn = 0;
double Pval_sredn = 0;
unsigned cnt_p_sredn = 0;

volatile unsigned NumCh_Aver = 0;

volatile bool Fl_0_Before = false;
volatile double cnt_time_0_before = 0.0;
volatile double Sum0Before = 0.0;
volatile double Time0Before = 10.0;

volatile bool Fl_0_After = false;
volatile double cnt_time_0_after = 0.0;
volatile double Sum0After = 0.0;
volatile double Time0After = 10.0;

//Calc
volatile double M_val = 0;
volatile double Pval = 0;
volatile double Pgd = 0;

//Define
volatile double Kusil = 1.0;
volatile double Msopr = 1.0;
volatile double KPD_Red = 0.5;
volatile double Per_Otnosh = 1.0;

volatile unsigned Refresh_Cycle_View = 2; //+
volatile unsigned cnt_cycle_view = 0; //+
volatile unsigned num_block_view = N_RCV_DEF; //+

typedef volatile struct
{
    bool Fl_Connect; 	//+
    bool Fl_Start;		//+
    bool Fl_SD_Ready;
    bool Reg_On;	//+
    bool Mode_UDP;      //else MODE = COM_Port
    bool fl_rcv_data_en;
    bool fl_rcv_control_en;

    unsigned int N_RCV; //+
    unsigned int cnt_recv; //+
    unsigned int cnt_obr;	//+
    unsigned int cnt_byte_rec;
    unsigned int cnt_cdg_recv;
    unsigned int cnt_block_recv; //+
    unsigned int  Num_Ch;	//+
    unsigned int cnt_reg_recv; //+

    double Fd; //+
    double Fd_Inv; //+

    double  mx_ch[3]; //+
    double  min_ch[3]; //+
    double  k_norm[3]; //????
    double  obr_buf_f[3][N_RCV_MAX]; //+
	
//-------------
	
	unsigned char recv_buf[N_RCV_MAX]; //+
	short obr_buf[N_RCV_MAX]; //+
	
	bool fl_write_to_file; //+	
	unsigned cnt_data_recv; //+	
	
	unsigned Refresh_Cycle_View;
	unsigned cnt_cycle_view;
	unsigned num_block_view = N_RCV_DEF;
	
}data_managing;

typedef volatile struct{

	bool Tahometr_On;
	bool Fl_Porog;
	double PorogTah;
	unsigned NumChTah;
	unsigned NumChVol;
	unsigned CntObTah;
	unsigned ObMinTah_Aver;
	unsigned TimeObTah;
	double Sum_Tah;
	unsigned CntAverTah;
	double TimeAverTah;
	unsigned SumTimeAverTah;
	double win_mean_p[NUM_MEAN_P];
	unsigned short cnt_mean;
	double TimeAverVol;
	double Sum_Vol;
	double Aver_Vol;
	unsigned CntAverVol;
	unsigned Detect_Tah;
	double Pgd_sredn;
	double Pval_sredn;
	unsigned cnt_p_sredn;

	unsigned NumCh_Aver;

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

}torsi_control;

 
typedef volatile struct
{
    uint32_t cnt_status_write;
    uint32_t cnt_file_sd;
    uint32_t cnt_status_write_all;
    uint8_t stat[4];

    uint16_t data[SIZE_DATA_CDG];

}cdg_control_sd;


#endif // WADC_DEFINES_H
