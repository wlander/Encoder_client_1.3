/**
 * @file fwriter.h
 * @brief Real-Time data writting to file with buffering
 * @version V1.0.0
 * @author suskov.e
 * @date 21.05.2021
 */

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QByteArray>
#include <fwriter.h>

void fwriter::proc(const QByteArray in_data)
{

    if((!file_path.isEmpty())){

         if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
             file.write(in_data.data(), in_data.size());
             file.close();
              //qDebug("Write in file is completed!");
             fwriter_ready();
         }
         else{
             qDebug("error file open!");
         }


    }
    else{
        qDebug("file path is empty!");
    }

}

int fwriter::set_file(QString fil){

    if(!fil.isEmpty()){
        file_path.clear();
        file_path.append(fil);
        file.setFileName(fil);
        fwriter_ready();
    }
    else{
        return 1;
    }

    return 0;
}
