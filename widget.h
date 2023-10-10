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
#include <QValueAxis>
#include <QScatterSeries>
#include <QSplineSeries>


#include <QRandomGenerator>

#include "chart.h"
#include "chartview.h"
#include "chartview_move.h"
#include "moveitem.h"
#include "getparams.h"
#include <QVXYModelMapper>
#include <QAbstractTableModel>

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
    unsigned char freq_msb;
    unsigned char freq_lsb;
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
    bool useFuncFlag;
    bool useTstFlag;

    size_t rdSet_num;
    QTimer *timer, *timerReq;
    size_t secNum, currSec;

    QMutex mutex;
    QFile fl, fl_tmp;

    //ручной режим
    QChart *chart;
    QScatterSeries *sDots;
    QLineSeries *sLine;

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

    QGraphicsScene *scene;

    QVector<MoveItem *>itVec;

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

    static int randomBetween(const int &low, const int &high);

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
    void slot_sendCurrLimits();

    void slot_manualAdjMode();


    //COM port
    void readRawData();
    void handleError(QSerialPort::SerialPortError error);
    //timers
    void updateTime();
    void sendReq();
public slots:
    void handleMarkerClicked();
    void slot_repaintChart( const QVector<QPointF> &vect );
signals:
    void signal_outMsgWithData( QString str );
    //COM port
    void signalSaveByteArray(QByteArray tmp);

};




#endif // WIDGET_H
