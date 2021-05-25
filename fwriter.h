/**
 * @file fwriter.h
 * @brief Real-Time data writting to file with buffering
 * @version V1.0.0
 * @author suskov.e
 * @date 21.05.2021
 */

#ifndef FWRITER_H
#define FWRITER_H

#include <QtCore>

#define DBLOCK  67108864 //size buffer of data

typedef volatile struct{
    unsigned volatile int cnt_wr;
    unsigned volatile int cnt_rd;
    unsigned volatile int cur_wr;
    unsigned volatile int cur_rd;
    char* data;  //pointer for circlar data buffer
}data_control;

/**
 * @brief class for shared data write/read access with lock by mutex
*/

class data_rep{
public:
    data_rep();
    int dwrite(char*, unsigned int);
    int fwrite(void);
    int set_file(QString);
private:
    QMutex dMutex;
    data_control* d_cntrl;
    QFile file;
    QString file_path;
    Q_DISABLE_COPY(data_rep)
};

/**
 * @brief base class for read/write
 * @param rwriter(data_rep& dr) -constuctor with data_rep ref
 * @param rwrite() - virtual function that need overwrite for definition of function for child classes
*/

class rwriter{
Q_OBJECT
public:
    rwriter(data_rep& dr);
    virtual ~rwriter(){}
    void stop();
protected:
    virtual void proc();
    int stat;
};

/**
 * @brief class for recieve data and writting these ones to shared buffer
*/

class dwriter : public rwriter{
Q_OBJECT
public:
    dwriter(data_rep& dr): rwriter(dr){}
protected:
    virtual void proc();
};

/**
 * @brief class for read data and writting these ones to file
*/

class fwriter : public rwriter{
Q_OBJECT
public:
    fwriter(data_rep& dr): rwriter(dr){}
protected:
    virtual void proc();
private:


};


#endif // FWRITER_H
