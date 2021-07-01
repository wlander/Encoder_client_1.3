/**
 * @file fwriter.h
 * @brief Real-Time data writting to file with buffering
 * @version V1.0.0
 * @author suskov.e
 * @date 21.05.2021
 */

#ifndef FWRITER_H
#define FWRITER_H


#include <QObject>
#include <QByteArray>
#include <QFile>

/**
 * @brief class for shared data write/read access with lock by mutex
*/

class fwriter : public QObject{

    Q_OBJECT

public:
    explicit fwriter(QObject *parent = 0){}
    ~fwriter(){}
    int set_file(const QString);
public slots:
    void proc(const QByteArray);
signals:
    void fwriter_ready();
private:
    QFile file;
    QString file_path;
};

#endif // FWRITER_H
