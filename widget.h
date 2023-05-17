#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QTime>
#include <QTimer>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QFile>
#include <QMessageBox>

#include "CONST_VAL.h"
#include "comPort.h"
#include "threadClass.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class SerialPort;
class threadClass;

#pragma pack(push, 1)
typedef struct wr_st_t {
    unsigned char strt;
    unsigned char mode;
    unsigned char pulse;
    uint16_t freq;
    unsigned char rsrv;
    uint16_t chSm;
}wrStruct;
#pragma pack(pop)

typedef union snd_pckg_t {
    unsigned char msgMas[sizeof(wrStruct)];
    wrStruct wrs;
}wrCmdMsg;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    SerialPort *serPort;
    threadClass *thC;
    QMenu *flMenu;
    QAction *actClean;
    wrCmdMsg msgCmd;
    QTime time;

    size_t rdSet_num;
    QTimer *timer;
    size_t secNum, currSec;

    QMutex mutex;
    QFile fl, fl_tmp;

    int val=0;
    int tmp=0;
    int cntr=0;
    int numInArr=0;

    //QVector <char> vecRawData;
    QVector <QByteArray> vecRawData;
    QVector<char> iDataV;
    QVector<QByteArray> qDataV;
    QVector<double> params;
    QVector<QVector<double>>points;


    int iCnt;

    void updateComInfo();
    void sortAlphabetically();
    void formAndSndMsg(const unsigned char & mode, const unsigned char & pulse, const uint16_t & freq);
    void startTest();
    void stopTest( bool byBtn );
    void parseResult();
    double countValues( const uint16_t & v );
    //void parseData( QVector<char> & vect );

private slots:    
    void slot_connectToCom();
    void slot_manageTest();
    void slot_cleanScreen();
    void slot_setConnection();
    void slot_stopConnection();
    void slot_outMsgWithData(const QString &str );

    void slot_2(const QByteArray &str );
    void slotSaveByteArray( const QByteArray & arr);

    void updateTime();
signals:
    void signal_setConnection();
    void signal_stopConnection();
    void signal_outMsgWithData( QString str );
    void signal_wrData(QByteArray data);
    void signalStopThread();
    void signalForThread( QVector<char>v);

};




#endif // WIDGET_H
