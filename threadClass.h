#ifndef THREADCLASS_H
#define THREADCLASS_H

#include <QThread>
#include <QFile>
#include <QDir>

class threadClass : public QThread {
    Q_OBJECT
public:
    threadClass( QObject *parent) : QThread(parent),
                             numInArr(0), cntr(0), val(0), tmp(0), started(true) {
        fl_tmp.setFileName(QDir::currentPath()+"\\res.txt");
        fl_tmp.open(QIODevice::WriteOnly);

    }


private:
    void run() override;
    QVector <char> dataVec;
    int numInArr;
    int cntr;
    int val, tmp;
    QVector<int>params;
    QVector<QVector<int>>points;
    bool started;

    QFile fl_tmp;

public slots:
    void stopAllThreads() { started = false;}
    void getDataForParse( QVector<char> inputVec) {
        dataVec = inputVec;
    }


};



#endif // THREADCLASS_H
