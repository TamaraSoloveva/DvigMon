#ifndef THREADCLASS_H
#define THREADCLASS_H

#include <QThread>

class threadClass : public QThread {
    Q_OBJECT
public:
    threadClass( QObject *parent) : QThread(parent),
                             numInArr(0), cntr(0), val(0), tmp(0), started(true) { }


private:
    void run() override;
    QVector <char> dataVec;
    int numInArr;
    int cntr;
    int val, tmp;
    QVector<int>params;
    QVector<QVector<int>>points;
    bool started;

public slots:
    void stopAllThreads() { started = false;}
    void getDataForParse( QVector<char> inputVec) {
        dataVec = inputVec;
    }


};



#endif // THREADCLASS_H
