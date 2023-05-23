#include "widget.h"
#include "ui_widget.h"



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), timer(nullptr), secNum(0), currSec(0), seriesI0(nullptr),
        seriesI1(nullptr), seriesI2(nullptr), seriesU(nullptr),  iCnt(0),  m_serial(new QSerialPort(this))
{
    ui->setupUi(this);
    updateComInfo();

    flMenu = new QMenu;
    actClean = new QAction("Clean screen", ui->textEdit);
    ui->textEdit->addAction(actClean);
    ui->textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->lineEdit->setValidator( new QIntValidator(1, 1000000000));
    ui->lineEdit_2->setValidator( new QIntValidator(0, 100));
    ui->lineEdit_3->setValidator( new QIntValidator(0, 60));

    connect(actClean, &QAction::triggered, this, &Widget::slot_cleanScreen);
    //"Connect"
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Widget::slot_connectToCom);
    //"Start test"
    connect(ui->pushButton, &QPushButton::clicked, this, &Widget::slot_manageTest);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &Widget::slot_ParseResult);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &Widget::slot_sendData);
    connect(this, &Widget::signal_outMsgWithData, this, &Widget::slot_outMsgWithData);

    //COM port
    connect( m_serial, &QSerialPort::readyRead, this, &Widget::readRawData);
    connect( m_serial, &QSerialPort::errorOccurred, this, &Widget::handleError);

    QMenu *pmnu   = new QMenu("&Menu");
    pmnu->addAction("&Save charts", this,  SLOT(slot_saveCharts()), Qt::CTRL + Qt::Key_S);
    QMenuBar *mnuBar;
    mnuBar = new QMenuBar();
    ui->gridLayout->addWidget(mnuBar);
    mnuBar->addMenu(pmnu);

    val=tmp=cntr=numInArr=0;

    iDataV.clear();

    // Создаём представление графиков
    chartViewI0 = new QChartView(this);
    QVBoxLayout *vbox0 = new QVBoxLayout;
    vbox0->addWidget(chartViewI0);
    ui->tab_4->setLayout(vbox0);
    chartI0 = new QChart();
    chartI0->legend()->hide();
    chartI0->createDefaultAxes();
    chartI0->setTitle("I0");
    chartViewI0->setChart(chartI0);


    chartViewI1 = new QChartView(this);
    QVBoxLayout *vbox1 = new QVBoxLayout;
    vbox1->addWidget(chartViewI1);
    ui->tab_5->setLayout(vbox1);
    chartI1 = new QChart();
    chartI1->legend()->hide();
    chartI1->createDefaultAxes();
    chartI1->setTitle("I1");
    chartViewI1->setChart(chartI1);

    chartViewI2 = new QChartView(this);
    QVBoxLayout *vbox2 = new QVBoxLayout;
    vbox2->addWidget(chartViewI2);
    ui->tab_3->setLayout(vbox2);
    chartI2 = new QChart();
    chartI2->legend()->hide();
    chartI2->createDefaultAxes();
    chartI2->setTitle("I2");
    chartViewI2->setChart(chartI2);

    chartViewU = new QChartView(this);
    QVBoxLayout *vbox3 = new QVBoxLayout;
    vbox3->addWidget(chartViewU);
    ui->tab_2->setLayout(vbox3);
    chartU = new QChart();
    chartU->legend()->hide();
    chartU->createDefaultAxes();
    chartU->setTitle("U");
    chartViewU->setChart(chartU);
}

void Widget::readRawData() {
    QByteArray rdData = m_serial->readAll();
    SaveByteArray(rdData );
}


void Widget::slot_sendData() {
    if ( ui->lineEdit_2->text().isEmpty() || ui->lineEdit_3->text().isEmpty() ) {
        emit signal_outMsgWithData("Error - Input range and frequency");
        return;
    }

    int r =  ui->lineEdit_2->text().toInt() ;
    int f =  ui->lineEdit_3->text().toInt() ;
    if (( r < 0) || ( r > 100)) {
        emit signal_outMsgWithData("Error in range");
        return;
    }


    if (( f < 0) || ( f > 60)) {
        emit signal_outMsgWithData("Error in frequency");
        return;
    }

    msgCmd.wrs.strt = '@';
    msgCmd.wrs.range = (unsigned char)r;
    msgCmd.wrs.freq = (unsigned char)f;
    msgCmd.wrs.end = '!';
    writeSerialPort(msgCmd);
    emit signal_outMsgWithData("Send " + QString::number(r, 10) + " range and "  + QString::number(f, 10) +  " frequency");
}


void Widget::slot_saveCharts() {
    QPixmap p1 = chartViewI0->grab();
    p1.save("i1.png", "PNG");

    QPixmap p2 = chartViewI1->grab();
    p2.save("i2.png", "PNG");

    QPixmap p3 = chartViewI2->grab();
    p3.save("i3.png", "PNG");

    QPixmap pU = chartViewU->grab();
    pU.save("u.png", "PNG");

    emit signal_outMsgWithData("Charts was saved to " + QDir::currentPath() );
}



void Widget::slot_manageTest() {
    if ( ui->pushButton->text() == "Start test") {
        startTest();
    }
    else {
        stopTest(true);
    }
}


double Widget::countValues( const uint16_t & v ) {
    return ((double) v * 0.076 - 2500);
}

void Widget::slot_ParseResult() {
    QString filename = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Document"), QDir::currentPath(), QObject::tr("Document files (*.txt);;All files (*.*)"));
    if (!filename.isEmpty()) {
        QFile fl(filename);
        if (!fl.open(QIODevice::ReadOnly) ) {
           QMessageBox::critical(nullptr, "Error", "Unable to open file", QMessageBox::Ok);
        }
        uint8_t ch=0;
        bool isMsg = false;
        uint8_t tmp = 0;
        uint16_t wVal=0;
        double dwVal = 0;
        QByteArray line;
        points.clear();
        params.clear();

        while (! fl.atEnd()) {
            line = fl.readLine();
            const size_t count = line.size();
            unsigned char* hex =new unsigned char[count];
            memcpy(hex, line.constData(), count);
            for (size_t i=0; i<count; ++i) {
                ch = hex[i];
                if (ch == '@') {
                    isMsg = true;
                    iCnt=1;
                    params.clear();
                    tmp = 0;
                    continue;
                }

                if ((ch == '!') && (iCnt != 11)) {
                    isMsg = false;
                    iCnt=0;
                    params.clear();
                    tmp = 0;
                    continue;
                }

                if (iCnt > 11) {
                    isMsg = false;
                    iCnt=0;
                    params.clear();
                    tmp = 0;
                    continue;
                }

                if ( isMsg ) {
                    switch(iCnt) {
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                        tmp = ch;
                        break;
                    case 2:
                    case 4:
                    case 6:
                    case 8:
                        wVal = ch;
                        wVal <<= 8;
                        wVal += (uint16_t)tmp;
                        dwVal = countValues(wVal);
/*                        if ((dwVal > -771) && (dwVal < -770)) {
                            int t=0;
                        }*/
                        params.push_back(dwVal);
                        break;
                    case 9:
                    case 10:
                        break;
                    case 11:
                        if (ch == '!') {
                            points.push_back(params);
                        }
                        iCnt=0;
                        params.clear();
                        isMsg = false;
                        continue;
                    }
                }
                iCnt++;
            }
            delete[]hex;
       }


        if (points.isEmpty()) {
            fl.close();
            emit signal_outMsgWithData("Bad file");
            return;

        }

        //построение графиков
        double min=0, max=0;
        if (seriesI0) delete seriesI0;
        seriesI0 = new QLineSeries();
        min = points.at(0).at(0);
        max = points.at(0).at(0);
        for (int i=0; i<points.size(); ++i){
            seriesI0->append(i, points.at(i).at(0));
            if ( points.at(i).at(0) < min) min = points.at(i).at(0);
            if ( points.at(i).at(0) > max) max = points.at(i).at(0);
        }
        chartI0->removeAllSeries();
        chartI0->addSeries(seriesI0);
        chartI0->createDefaultAxes();
        chartI0->axes(Qt::Horizontal).first()->setRange(0, points.size() );
        chartI0->axes(Qt::Vertical).first()->setRange((int)min, (int)max);

        if (seriesI1) delete seriesI1;
        seriesI1 = new QLineSeries();
        min = points.at(0).at(1);
        max = points.at(0).at(1);
        for (int i=0; i<points.size(); ++i){
             seriesI1->append(i, points.at(i).at(1));
             if ( points.at(i).at(1) < min)
                 min = points.at(i).at(1);
             if ( points.at(i).at(1) > max) max = points.at(i).at(1);
         }
        chartI1->removeAllSeries();
        chartI1->addSeries(seriesI1);
        chartI1->createDefaultAxes();
        chartI1->axes(Qt::Horizontal).first()->setRange(0, points.size() );
        chartI1->axes(Qt::Vertical).first()->setRange((int)min, (int)max);

        if (seriesI2) delete seriesI2;
        seriesI2 = new QLineSeries();
        min = points.at(0).at(2);
        max = points.at(0).at(2);
        for (int i=0; i<points.size(); ++i){
            seriesI2->append(i, points.at(i).at(2));
            if ( points.at(i).at(2) < min) min = points.at(i).at(2);
            if ( points.at(i).at(2) > max) max = points.at(i).at(2);
        }
        chartI2->removeAllSeries();
        chartI2->addSeries(seriesI2);
        chartI2->createDefaultAxes();
        chartI2->axes(Qt::Horizontal).first()->setRange(0, points.size() );
        chartI2->axes(Qt::Vertical).first()->setRange((int)min, (int)max);

        if (seriesU) delete seriesU;
        seriesU = new QLineSeries();
        min = points.at(0).at(3);
        max = points.at(0).at(3);
        for (int i=0; i<points.size(); ++i){
            seriesU->append(i, points.at(i).at(3));
            if ( points.at(i).at(3) < min) min = points.at(i).at(3);
            if ( points.at(i).at(3) > max) max = points.at(i).at(3);
        }
        chartU->removeAllSeries();
        chartU->addSeries(seriesU);
        chartU->createDefaultAxes();
        chartU->axes(Qt::Horizontal).first()->setRange(0, points.size() );
        chartU->axes(Qt::Vertical).first()->setRange((int)min, (int)max);


        emit signal_outMsgWithData("Charts from file "+filename+" ready");
        fl.close();

    }
    else
         emit signal_outMsgWithData("File is empty");

}


void Widget::slot_cleanScreen() {
    ui->textEdit->clear();
}

Widget::~Widget() {
    if (seriesI0) delete seriesI0;
    delete chartI0;
    delete chartViewI0;

    if (seriesI1) delete seriesI1;
    delete chartI1;
    delete chartViewI1;

    if (seriesI2) delete seriesI2;
    delete chartI2;
    delete chartViewI2;


    if (seriesU) delete seriesU;
    delete chartU;
    delete chartViewU;


    slot_stopConnection();
    delete ui;
}

void Widget::slot_outMsgWithData( const QString &str ) {    
    QString sOut = time.currentTime().toString();
    sOut = "\n"+sOut + " " + str;
    ui->textEdit->append(sOut);
}


void Widget::slot_connectToCom() {    
    if (ui->pushButton_2->text() == "Disconnect")
        slot_stopConnection();
    else
        slot_setConnection();
}


void Widget::slot_stopConnection() {
    emit signal_outMsgWithData("Disconnected from COM-port");
    ui->pushButton_2->setText("Connect");
    ui->comboBox->setEnabled(true);
    ui->pushButton->setEnabled(false);
    closeSerialPort();
    if (timer) {
        stopTest(true);
        timer = nullptr;
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
   if (openSerialPort() == -1)
       return;
   ui->pushButton_2->setText("Disconnect");
   ui->pushButton->setEnabled(true);
   ui->comboBox->setEnabled(false);
}

int Widget::openSerialPort() {
    m_serial->setPortName(ui->comboBox->currentText());
    m_serial->setBaudRate(115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        emit signal_outMsgWithData("Connected to COM-port");
        return 0;

    }
    else {
        emit signal_outMsgWithData("Error - " +  m_serial->errorString() );
        return -1;
    }
}

void Widget::closeSerialPort() {
    if (m_serial->isOpen())
        m_serial->close();
}

void Widget::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        emit signal_outMsgWithData("Error - " +  m_serial->errorString() );
        slot_stopConnection();
    }
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

    msgCmd.wrs.strt = '@';
    msgCmd.wrs.range = '!';
    msgCmd.wrs.freq = '@';
    writeSerialPort(msgCmd, 3);
}


void Widget::writeSerialPort( wrCmdMsg & msgCmd, size_t sz ) {
    if (!sz) sz = sizeof(msgCmd.msgMas);
    QByteArray data = QByteArray(reinterpret_cast< char *>(msgCmd.msgMas), sz);
    if (m_serial->isOpen()) m_serial->write(data);
}

void Widget::stopTest(bool byBtn) {
     timer->stop();
     delete timer;
     timer = nullptr;
     ui->pushButton->setText("Start test");
     if (byBtn) {
        emit signal_outMsgWithData(QString("Test interrupted. Work time: %1 seconds").arg(currSec));       
     }
     else {
        emit signal_outMsgWithData("Write file finished.");
     }
     if (fl.isOpen()) {
         fl.flush();
         fl.close();
     }
     msgCmd.wrs.strt = '@';
     msgCmd.wrs.range = '?';
     msgCmd.wrs.freq = '@';
     writeSerialPort(msgCmd, 3);
}

void Widget::updateTime() {
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
        //emit signalForThread(iDataV);
    }
    currSec++;

    if (secNum == currSec) {
        stopTest(false);
    }
}


void Widget::SaveByteArray( const QByteArray &arr) {
    fl.write(arr);
    fl.flush();
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
