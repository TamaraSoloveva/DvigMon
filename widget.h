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
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMenuBar>
#include <QDir>
#include <QFileDialog>
#include <QIODevice>
#include <QObject>

#include "chart.h"
#include "chartview.h"

QT_BEGIN_NAMESPACE
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QLegendMarker>
QT_END_NAMESPACE

#define CH1_LEG "до фильтрации"
#define CH2_LEG "медианный"
#define CH3_LEG "бегущее среднее"



#define NUM_READ 10 // порядок медианы


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

#pragma pack(push, 1)
typedef struct wr_st_t {
    unsigned char strt;
    unsigned char range;
    unsigned char freq;
    unsigned char end;
}wrStruct;
#pragma pack(pop)

typedef union snd_pckg_t {
    unsigned char msgMas[sizeof(wrStruct)];
    wrStruct wrs;
}wrCmdMsg;

QT_CHARTS_USE_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
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
    bool zeroCycle;
    QVector<float>shiftVec;
    QLegendMarker *markI0, *markIMed, *markIk;

    QVector <QByteArray> vecRawData;
    QVector<char> iDataV;
    QVector<QByteArray> qDataV;
    QVector<float> params;
    QVector<QVector<float>>points;

    ChartView *chartViewI0;
    Chart *chartI0;
    QLineSeries *seriesI0;
    QLineSeries *seriesKI0;
    QLineSeries *seriesMedI0;

    ChartView *chartViewI1;
    QChart *chartI1;
    QLineSeries *seriesI1;
    QLineSeries *seriesKI1;
    QLineSeries *seriesMedI1;

    ChartView *chartViewI2;
    Chart *chartI2;
    QLineSeries *seriesI2;
    QLineSeries *seriesKI2;
    QLineSeries *seriesMedI2;

    ChartView *chartViewU;
    Chart *chartU;
    QLineSeries *seriesU;
    QLineSeries *seriesKU;
    QLineSeries *seriesMedU;

    int iCnt;
    QSerialPort *m_serial;

    void updateComInfo();
    void sortAlphabetically();
    void startTest();
    void stopTest( bool byBtn );
    float countValues( const uint16_t & v );
    int openSerialPort();
    void closeSerialPort();
    void writeSerialPort( wrCmdMsg & msgCmd, size_t sz = 0 );
    void SaveByteArray( const QByteArray & arr);
    float findMedianN_optim(const float & newVal);
    void printCharts( const QVector<QVector<float>> &points, const float  & k);

private slots:    
    void slot_connectToCom();
    void slot_manageTest();
    void slot_cleanScreen();
    void slot_setConnection();
    void slot_stopConnection();
    void slot_outMsgWithData(const QString &str );
    void slot_ParseResult();
    void slot_saveCharts();
    void slot_sendData();

    //COM port
    void readRawData();
    void handleError(QSerialPort::SerialPortError error);
    void updateTime();
public slots:
    void handleMarkerClicked();
signals:
    void signal_outMsgWithData( QString str );
    //COM port
    void signalSaveByteArray(QByteArray tmp);

};




#endif // WIDGET_H
