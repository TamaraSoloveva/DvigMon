#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMenuBar>
#include <QChart>
#include <QChartView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QDir>
#include <QFileDialog>

#define PACK_SIZE 10

QT_CHARTS_USE_NAMESPACE

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), serPort(nullptr), thC(nullptr), rdSet_num(0), timer(nullptr), secNum(0), currSec(0)
{
    ui->setupUi(this);
    updateComInfo();

    flMenu = new QMenu;
    actClean = new QAction("Clean screen", ui->textEdit);
    ui->textEdit->addAction(actClean);
    ui->textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->lineEdit->setValidator( new QIntValidator(1, 1000000000));
    ui->lineEdit->setText( QString::number(10));


    connect(actClean, &QAction::triggered, this, &Widget::slot_cleanScreen);
    //"Connect"
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Widget::slot_connectToCom);
    //"Start test"
    connect(ui->pushButton, &QPushButton::clicked, this, &Widget::slot_manageTest);
    connect(this, &Widget::signal_setConnection, this, &Widget::slot_setConnection);
    connect(this, &Widget::signal_stopConnection, this, &Widget::slot_stopConnection);
    connect(this, &Widget::signal_outMsgWithData, this, &Widget::slot_outMsgWithData);

   val=tmp=cntr=numInArr=0;

    iDataV.clear();



    // Создаём представление графиков
    QChartView *chartViewI0 = new QChartView(this);


    ui->horizontalLayout_2->addWidget(chartViewI0);

         QLineSeries *series = new QLineSeries();
            series->append(0, 6);
            series->append(2, 4);
            series->append(3, 8);
            series->append(7, 4);
            series->append(10, 5);
            *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) <<   QPointF(20, 2);
            QChart *chartI0 = new QChart();
            chartI0->legend()->hide();
            chartI0->addSeries(series);
            chartI0->createDefaultAxes();
            chartI0->setTitle("Curr I0");
            chartViewI0->setChart(chartI0);

            QChartView *chartViewI1 = new QChartView(this);
            ui->horizontalLayout_2->addWidget(chartViewI1);

            QChart *chartI1 = new QChart();
            chartI1->legend()->hide();
            chartI1->createDefaultAxes();
            chartI1->setTitle("Curr I1");
            chartViewI1->setChart(chartI1);


           QChartView *chartViewI2 = new QChartView(this);
           ui->horizontalLayout_3->addWidget(chartViewI2);

           QChart *chartI2 = new QChart();
           chartI2->legend()->hide();
  //         chartI1->addSeries(series);
         chartI2->createDefaultAxes();
           chartI2->setTitle("Curr I2");
          chartViewI2->setChart(chartI2);

          QChartView *chartViewU = new QChartView(this);
          ui->horizontalLayout_3->addWidget(chartViewU);

          QChart *chartU = new QChart();
          chartU->legend()->hide();
 //         chartI1->addSeries(series);
        chartU->createDefaultAxes();
          chartU->setTitle("U");
         chartViewU->setChart(chartU);
}

void Widget::slot_manageTest() {
    if ( ui->pushButton->text() == "Start test") {
        startTest();
    }
    else if ( ui->pushButton->text() == "Get result") {
        parseResult();
    }
    else {
        stopTest(true);
    }
}


uint32_t Widget::countValues( const uint16_t & v ) {
    return (v * 0x076 - 2500);
}

void Widget::parseResult() {
    QString filename = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Document"), QDir::currentPath(), QObject::tr("Document files (*.txt);;All files (*.*)"));
    if (!filename.isEmpty()) {
        QFile fl(filename);
        if (!fl.open(QIODevice::ReadOnly) ) {
           QMessageBox::critical(nullptr, "Error", "Unable to open file", QMessageBox::Ok);
        }


        char ch;
        bool isMsg = false;
        int iCnt=0;
        char tmp;
        uint16_t wVal;
        uint32_t dwVal;
        QString aaa;
        QByteArray line;
        unsigned char* buffer;
        const unsigned char *data;
        while (! fl.atEnd()) {
            line = fl.readLine();

            data = reinterpret_cast<const unsigned char*>(line.data());




  /*          if (ch == '@') {
                isMsg = true;
                iCnt++;
                continue;
            }
            if ( isMsg ) {
                switch(iCnt) {
                case 1:
                case 3:
                case 5:
                case 7:
                case 9:
                    tmp = ch;
                    break;
                case 2:
                case 4:
                case 6:
                case 8:
                case 10:
                    wVal = ch;
                    wVal <<= 8;
                    wVal += tmp;
                    dwVal = countValues(wVal);


                    tmp = 0;
                    break;
                case 11:
                    if (ch == '!') {
                      /*  points.push_back(params);
                        params.clear();*/
               /*     }
                    else {
                      //  emit signal_outMsgWithData("Bad pack");

                    }
                    params.clear();
                    cntr=0;
                    continue;
                }
                iCnt++;

            }*/
        }
    }

}



void Widget::formAndSndMsg(const unsigned char &mode, const unsigned char &pulse, const uint16_t &freq) {
    unsigned int i=0;
    msgCmd.wrs.strt = 0x35;
    msgCmd.wrs.mode = mode;
    msgCmd.wrs.pulse = pulse;
    msgCmd.wrs.freq = freq;
    msgCmd.wrs.rsrv = 0;

    uint16_t chSm=0;

    for (i=0; i<sizeof(wr_st_t)-2; ++i){
        chSm += msgCmd.msgMas[i];
    }
    msgCmd.wrs.chSm = chSm;
    QByteArray data = QByteArray(reinterpret_cast<char *>(msgCmd.msgMas), sizeof(msgCmd.msgMas));
    emit signal_wrData(data);
}

void Widget::slot_cleanScreen() {
    ui->textEdit->clear();
}

Widget::~Widget()
{
    slot_stopConnection();
    delete ui;
}

void Widget::slot_outMsgWithData( const QString &str ) {
    QString sOut = time.currentTime().toString();
    sOut = sOut + " " + str;
    ui->textEdit->append(sOut);
}

void Widget::slot_2( const QByteArray &str ) {
    ui->textEdit->append(str);
}


void Widget::slot_connectToCom() {
    if (ui->pushButton_2->isChecked())
        emit signal_stopConnection();
    else
        emit signal_setConnection();
}


void Widget::slot_stopConnection() {
    emit signal_outMsgWithData("Disconnected from COM-port");
    ui->pushButton_2->setText("Connect");
    ui->comboBox->setEnabled(true);
    ui->pushButton->setEnabled(false);
    if (serPort) {
        delete serPort;
        serPort=nullptr;
    }
    if (timer) {
        stopTest(true);
        timer = nullptr;
    }
    if (thC && thC->isRunning()) {
        emit signalStopThread();
        thC = nullptr;
    }
    if (fl.isOpen()) {
        fl.flush();
        fl.close();
    }
}

void Widget::slot_setConnection() {
    if (ui->comboBox->currentText().isEmpty()) {
        emit signal_outMsgWithData("Error - Input COM-port number!!!");
        ui->pushButton_2->setChecked(true);
        return;
    }

    emit signal_outMsgWithData("Connected to COM-port");
    ui->pushButton_2->setText("Disconnect");
    ui->pushButton->setEnabled(true);
    ui->comboBox->setEnabled(false);

    try {
        serPort = new SerialPort(ui->comboBox->currentText(), 115200, QSerialPort::OneStop,
                           QSerialPort::Data8,  QSerialPort::NoParity );    
    }
    catch(std::bad_alloc &exc) {
        emit signal_outMsgWithData("Error - Allocation memory!!!");
    }

    connect(serPort, &SerialPort::signal_outMsgWithDataCom, this, &Widget::slot_outMsgWithData);
    connect(serPort, &SerialPort::signalSaveByteArray, this, &Widget::slotSaveByteArray);
    connect(this, &Widget::signal_wrData, serPort, &SerialPort::slot_writeData);
    connect(serPort, &SerialPort::sig, this, &Widget::slot_2);

}


void Widget::startTest() {
    if (ui->lineEdit->text().isEmpty()) {
        emit signal_outMsgWithData("Error - Input Time in seconds!!!");
        ui->pushButton_2->setChecked(true);
        return;
    }
    secNum = ui->lineEdit->text().toInt();
    currSec=0;
    ui->pushButton->setText("Stop test");
    emit signal_outMsgWithData(QString("Test started. Work time: %1 seconds").arg(secNum));

    timer = new QTimer;
    timer->setInterval(1000);
    timer->start();
    connect(timer, &QTimer::timeout, this, &Widget::updateTime);

    QString aa = QDir::currentPath()+"\\DATA.txt";
    fl.setFileName(aa);
    if (fl.open(QIODevice::WriteOnly)) {
        emit signal_outMsgWithData("log file " + aa + " opened");
    }
    else {
        emit signal_outMsgWithData("Unable to open log file ");
    }
}


void Widget::stopTest(bool byBtn) {
     timer->stop();
     delete timer;
     timer = nullptr;

     emit signalStopThread();
     if (byBtn) {
        emit signal_outMsgWithData(QString("Test interrupted. Work time: %1 seconds").arg(currSec));
         ui->pushButton->setText("Start test");
     }
     else {
        emit signal_outMsgWithData("Write file finished.");
         ui->pushButton->setText("Get result");
     }
     if (fl.isOpen()) {
         fl.flush();
         fl.close();
     }
}

void Widget::updateTime() {
    qDebug() << "timer\n";
    if (iDataV.isEmpty()) {
        QMutexLocker locker(&mutex);
        qDataV = vecRawData;
        vecRawData.clear();

        QByteArray tmpArr;
        for ( int i = 0; i<qDataV.size(); ++i ) {
            tmpArr = qDataV.at(i);
            for (int j=0; j < tmpArr.size(); ++j)
                iDataV.push_back(tmpArr.at(j));
        }
        emit signalForThread(iDataV);
    }
    currSec++;

    if (secNum == currSec) {
        stopTest(false);
    }
}


void Widget::slotSaveByteArray( const QByteArray &arr) {
    QMutexLocker locker(&mutex);
    vecRawData.push_back(arr);
    fl.write(arr);
}

void Widget::sortAlphabetically() {
    QStringList lst;
    int ttl_cnt =  ui->comboBox->count();
    if (ttl_cnt > 2) {
        for (int i=ttl_cnt-1; i != -1; i--) {
            lst << ui->comboBox->itemText(i);
            ui->comboBox->removeItem(i);
        }
        lst.sort();
        ui->comboBox->addItems(lst);
    }
}

void Widget::updateComInfo() {
    int ind=0;
    const auto serialPortInfoList = QSerialPortInfo::availablePorts();
    if (serialPortInfoList.count() !=  0) {
        QList<QSerialPortInfo>::const_iterator itr;
        itr = serialPortInfoList.begin();
        while (itr != serialPortInfoList.end()) {
            ui->comboBox->insertItem(ind, (*itr).portName()  );
            ind++;
            itr++;
        }
    }
    sortAlphabetically();
}
