/**
 * @file fwriter.c
 * @brief data writting to file from cach buffer
 * @author suskov.e
 * @date 20.05.2021
 */

#include "fwriter.h"
#include "QDataStream"

data_rep::data_rep():
    dMutex()
{
    d_cntrl = new data_control;
    d_cntrl->cnt_wr = 0;
    d_cntrl->cnt_rd = 0;
    d_cntrl->cur_wr = 0;
    d_cntrl->cur_rd = 0;
    d_cntrl->data = new char[DBLOCK];
}

int data_rep::set_file(QString fil){

    if(fil!=""){
        file_path = fil;
        file.setFileName(file_path);
    }
    else{
        return 1;
    }

    return 0;
}



int data_rep::dwrite(char* in_data, unsigned int n){

 unsigned int i;

    if(n<DBLOCK){

      if(!dMutex.tryLock()) return -1;

      for(i=0; i<n;i++){
          d_cntrl->data[d_cntrl->cur_wr++] = in_data[i];
          if(d_cntrl->cur_wr==DBLOCK) d_cntrl->cur_wr=0;
      }

      d_cntrl->cnt_wr+=n;

      dMutex.unlock();

    }
    else{
        return -1;
    }

    return 0;
}


int data_rep::fwrite(){
 unsigned int n = 0;
 unsigned int i;
 char* tmp_d;


    if(!dMutex.tryLock()) return -1;

    if(d_cntrl->cnt_wr!=d_cntrl->cnt_rd){
        n=d_cntrl->cnt_wr-d_cntrl->cnt_rd;
        if(n>0){
            tmp_d = new char[n];
            for(i=0; i<n;i++){
               tmp_d[i] = d_cntrl->data[d_cntrl->cur_rd++];
               if(d_cntrl->cur_rd==DBLOCK) d_cntrl->cur_rd=0;
            }
            d_cntrl->cnt_rd+=n;
        }
    }

    dMutex.unlock();

    if(n==0) return 1;
    if(file_path!="") return 2;
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        return 3;
    }
    else{
        file.write(tmp_d, n);
        file.close();
        delete [] tmp_d;
    }


   return 0;
}



void dwriter::proc()
{

}


void fwriter::proc()
{

}



/*
int data_rep::dwrite(char* in_data, unsigned int n){

 unsigned int tail = 0;
 unsigned int nn = n;
 unsigned int i;

    if(n<DBLOCK){

      while(!dMutex.tryLock());

      if((d_cntrl->cur_wr+n)>(DBLOCK-1)){
          tail = (d_cntrl->cur_wr+n)-DBLOCK;
          nn = n-tail;
      }

      for(i=0; i<nn;i++) d_cntrl->data[d_cntrl->cur_wr++] = in_data[i];

      if(tail>0){
          d_cntrl->cur_wr = 0;
          for(i=nn; i<n;i++) d_cntrl->data[d_cntrl->cur_wr++] = in_data[i];
      }

      d_cntrl->cnt_sum+=n;

      dMutex.unlock();

      return 0;

    }
    else{
        return -1;
    }

}
*/
