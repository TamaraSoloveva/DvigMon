#include "widget.h"
#include "ui_widget.h"



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), timer(nullptr), secNum(0), currSec(0), seriesI0(nullptr),
        seriesI1(nullptr), seriesI2(nullptr), seriesU(nullptr),  iCnt(0),  m_serial(new QSerialPort(this))
{
    ui->setupUi(this);
    updateComInfo();

    seriesKI0 = nullptr;
    seriesMedI0 = nullptr;
    seriesKI1 = nullptr;
    seriesMedI1 = nullptr;
    seriesKI2 = nullptr;
    seriesMedI2 = nullptr;
    seriesKU = nullptr;
    seriesMedU = nullptr;

    flMenu = new QMenu;
    actClean = new QAction("Clean screen", ui->textEdit);
    ui->textEdit->addAction(actClean);
    ui->textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->lineEdit->setValidator( new QIntValidator(1, 1000000000));
    ui->lineEdit_3->setValidator( new QIntValidator(0, 1000000000));

    connect(actClean, &QAction::triggered, this, &Widget::slot_cleanScreen);
    //"Connect"
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Widget::slot_connectToCom);
    //"Start test"
    connect(ui->pushButton, &QPushButton::clicked, this, &Widget::slot_manageTest);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &Widget::slot_ParseResult);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &Widget::slot_sendData);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &Widget::slot_sendCurrLimits);
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
    shiftVec.clear();
    zeroCycle = true;

    // Создаём представление графиков
    chartI0 = new Chart;
    chartI0->legend()->setAlignment(Qt::AlignBottom);
    chartI0->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chartI0->createDefaultAxes();
    chartI0->setTitle("I0");
    chartViewI0 = new ChartView(chartI0);
    QVBoxLayout *vbox0 = new QVBoxLayout;
    vbox0->addWidget(chartViewI0);
    ui->tab_4->setLayout(vbox0);   


    chartI1 = new Chart();
    chartI1->legend()->setAlignment(Qt::AlignBottom);
    chartI1->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chartI1->createDefaultAxes();
    chartI1->setTitle("I1");
    chartViewI1 = new ChartView(chartI1);
    QVBoxLayout *vbox1 = new QVBoxLayout;
    vbox1->addWidget(chartViewI1);
    ui->tab_5->setLayout(vbox1);

    chartI2 = new Chart();
    chartI2->legend()->setAlignment(Qt::AlignBottom);
    chartI2->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chartI2->createDefaultAxes();
    chartI2->setTitle("I2");   
    chartViewI2 = new ChartView(chartI2);
    QVBoxLayout *vbox2 = new QVBoxLayout;
    vbox2->addWidget(chartViewI2);
    ui->tab_3->setLayout(vbox2);

    chartU = new Chart();
    chartU->legend()->setAlignment(Qt::AlignBottom);
    chartU->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chartU->createDefaultAxes();
    chartU->setTitle("U");
    chartViewU = new ChartView(chartU);
    QVBoxLayout *vbox3 = new QVBoxLayout;
    vbox3->addWidget(chartViewU);
    ui->tab_2->setLayout(vbox3);


}

void Widget::handleMarkerClicked() {
    if (this->sender() == markI0) {
        if (seriesI0->isVisible())
            seriesI0->setVisible(false);
        else
            seriesI0->setVisible(true);
    }
    else if (this->sender() == markIMed) {
        if (seriesMedI0->isVisible())
            seriesMedI0->setVisible(false);
        else
            seriesMedI0->setVisible(true);
    }
    else {
        if (seriesKI0->isVisible())
            seriesKI0->setVisible(false);
        else
            seriesKI0->setVisible(true);
    }


}


void Widget::readRawData() {
    QByteArray rdData = m_serial->readAll();
    SaveByteArray(rdData );
}


void Widget::slot_sendData() {
    if ( ui->lineEdit_3->text().isEmpty() ) {
        emit signal_outMsgWithData("Error - Input frequency");
        return;
    }
    int f =  ui->lineEdit_3->text().toInt() ;
    msgCmd.wrs.strt = '@';
    msgCmd.wrs.freq_msb = (unsigned char)((f & 0x100) >> 8);
    msgCmd.wrs.freq_lsb = (unsigned char)f;
    msgCmd.wrs.end = '!';
    writeSerialPort(msgCmd);
    emit signal_outMsgWithData("Send  frequency "  + QString::number(f, 10) +  " Hz");
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


void Widget::slot_sendCurrLimits() {
    int fault_curr = 0, max_curr = 0;
    if (!ui->lineEdit_5->text().isEmpty())
        fault_curr = ui->lineEdit_5->text().toInt();
    if (!ui->lineEdit_6->text().isEmpty())
        max_curr = ui->lineEdit_6->text().toInt();
    msgCmd.wrs.strt = '@';
    msgCmd.wrs.freq_msb = (unsigned char)max_curr;
    msgCmd.wrs.freq_lsb = (unsigned char)fault_curr;
    msgCmd.wrs.end = '*';
    writeSerialPort(msgCmd);
    emit signal_outMsgWithData("Send fault current "  + QString::number(fault_curr, 10) +
                               " and max operation current " + QString::number(max_curr, 10));



}


void Widget::slot_manageTest() {
    if ( ui->pushButton->text() == "Start test") {
        startTest();
    }
    else {
        stopTest(true);
    }
}


float Widget::countValues( const uint16_t & v ) {
    return ((float) v * 0.076 - 2500);
}

void Widget::slot_ParseResult() {
    float k = ui->lineEdit_4->text().toFloat();
    if (k <= 0 || k > 1 ) {
        emit signal_outMsgWithData("k value - error");
        return;
    }
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
        float dwVal = 0;
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
        printCharts(points, k);
        emit signal_outMsgWithData("Charts from file "+filename+" ready");
        fl.close();
    }
    else
         emit signal_outMsgWithData("File is empty");

}

void Widget::printCharts( const QVector<QVector<float>> &points, const float &k) {
    QFile i0( QDir::currentPath()+"\\i0.txt");
    QFile i1(QDir::currentPath()+"\\i1.txt");
    QFile i2(QDir::currentPath()+"\\i2.txt");
    QFile u(QDir::currentPath()+"\\u.txt");
    i0.open( QIODevice::WriteOnly | QIODevice::Truncate );
    if (!i0.isOpen())  emit signal_outMsgWithData("Unable to open file " + i0.fileName());
    i1.open( QIODevice::WriteOnly | QIODevice::Truncate );
    if (!i1.isOpen())  emit signal_outMsgWithData("Unable to open file " + i1.fileName());
    i2.open( QIODevice::WriteOnly | QIODevice::Truncate );
    if (!i2.isOpen())  emit signal_outMsgWithData("Unable to open file " + i2.fileName());
    u.open( QIODevice::WriteOnly | QIODevice::Truncate );
    if (!u.isOpen())  emit signal_outMsgWithData("Unable to open file " + u.fileName());

    const int sz = points.size();
    float newVal = 0, newVals=0;

    shiftVec.clear();
    zeroCycle = true;
    float min=0, max=0;

//i0
    if (seriesI0) delete seriesI0;
    if (seriesKI0) delete seriesKI0;
    if (seriesMedI0) delete seriesMedI0;
    seriesI0 = new QLineSeries();
    seriesKI0 = new QLineSeries();
    seriesMedI0 = new QLineSeries();

    min = points.at(0).at(0);
    max = points.at(0).at(0);
    for (int i=0; i<sz; ++i){
        if (i0.isOpen()) {
            i0.write( QByteArray::number(points.at(i).at(0)));
            i0.write( " " );
        }
        seriesI0->append(i, points.at(i).at(0));
        newVal = findMedianN_optim(points.at(i).at(0));
        seriesMedI0->append(i, newVal);
        newVals += (newVal - newVals ) * k;
        seriesKI0->append(i, newVals);
        if ( points.at(i).at(0) < min) min = points.at(i).at(0);
        if ( points.at(i).at(0) > max) max = points.at(i).at(0);
    }
    chartI0->removeAllSeries();
    seriesI0->setName(CH1_LEG);
    seriesMedI0->setName(CH2_LEG);
    seriesKI0->setName(CH3_LEG);
    chartI0->addSeries(seriesI0);
    chartI0->addSeries(seriesMedI0);
    chartI0->addSeries(seriesKI0);

    markI0 = chartI0->legend()->markers()[1];
    markIMed = chartI0->legend()->markers()[1];
    markIk = chartI0->legend()->markers()[2];
    connect(markI0, &QLegendMarker::clicked, this, &Widget::handleMarkerClicked);
    connect(markIMed, &QLegendMarker::clicked, this, &Widget::handleMarkerClicked);
    connect(markIk, &QLegendMarker::clicked, this, &Widget::handleMarkerClicked);
    chartI0->createDefaultAxes();
    chartI0->axes(Qt::Horizontal).first()->setRange(0, sz );
    chartI0->axes(Qt::Vertical).first()->setRange((int)min, (int)max);
 //i1
    if (seriesI1) delete seriesI1;
    if (seriesKI1) delete seriesKI1;
    if (seriesMedI1) delete seriesMedI1;
    seriesI1 = new QLineSeries();
    seriesKI1 = new QLineSeries();
    seriesMedI1 = new QLineSeries();
    min = points.at(0).at(1);
    max = points.at(0).at(1);
    newVals = 0;
    newVal = 0;

    for (int i=0; i<sz; ++i){
        if (i1.isOpen()) {
            i1.write( QByteArray::number(points.at(i).at(1)));
            i1.write( " " );
        }
         seriesI1->append(i, points.at(i).at(1));
         newVal = findMedianN_optim( points.at(i).at(1));
         seriesMedI1->append(i, newVal);
         newVals += (newVal - newVals ) * k;
         seriesKI1->append(i, newVals);
         if ( points.at(i).at(1) < min) min = points.at(i).at(1);
         if ( points.at(i).at(1) > max) max = points.at(i).at(1);
     }
    chartI1->removeAllSeries();
    seriesI1->setName(CH1_LEG);
    seriesMedI1->setName(CH2_LEG);
    seriesKI1->setName(CH3_LEG);
    chartI1->addSeries(seriesI1);
    chartI1->addSeries(seriesMedI1);
    chartI1->addSeries(seriesKI1);
    chartI1->createDefaultAxes();
    chartI1->axes(Qt::Horizontal).first()->setRange(0, points.size() );
    chartI1->axes(Qt::Vertical).first()->setRange((int)min, (int)max);
//i2
    if (seriesI2) delete seriesI2;
    if (seriesKI2) delete seriesKI2;
    if (seriesMedI2) delete seriesMedI2;
    seriesI2 = new QLineSeries();
    seriesKI2= new QLineSeries();
    seriesMedI2 = new QLineSeries();
    min = points.at(0).at(2);
    max = points.at(0).at(2);
    newVal = 0;
    newVals = 0;
    for (int i=0; i<sz; ++i){
        if (i2.isOpen()) {
            i2.write( QByteArray::number(points.at(i).at(2)));
            i2.write( " " );
        }
        seriesI2->append(i, points.at(i).at(2));
        newVal = findMedianN_optim(points.at(i).at(2));
        seriesMedI2->append(i, newVal);
        newVals += (newVal - newVals ) * k;
        seriesKI2->append(i, newVals);
        if ( points.at(i).at(2) < min) min = points.at(i).at(2);
        if ( points.at(i).at(2) > max) max = points.at(i).at(2);
    }
    chartI2->removeAllSeries();
    seriesI2->setName(CH1_LEG);
    seriesKI2->setName(CH3_LEG);
    seriesMedI2->setName(CH2_LEG);
    chartI2->addSeries(seriesI2);
    chartI2->addSeries(seriesMedI2);
    chartI2->addSeries(seriesKI2);
    chartI2->createDefaultAxes();
    chartI2->axes(Qt::Horizontal).first()->setRange(0, points.size() );
    chartI2->axes(Qt::Vertical).first()->setRange((int)min, (int)max);
//u
    if (seriesU) delete seriesU;
    if (seriesKU) delete seriesKU;
    if (seriesMedU) delete seriesMedU;
    seriesU = new QLineSeries();
    seriesKU= new QLineSeries();
    seriesMedU = new QLineSeries();
    min = points.at(0).at(3);
    max = points.at(0).at(3);
    newVal = 0;
    newVals = 0;
    for (int i=0; i<sz; ++i){
        if (u.isOpen()) {
            u.write( QByteArray::number(points.at(i).at(3)));
            u.write( " " );
        }
        seriesU->append(i, points.at(i).at(3));
        newVal = findMedianN_optim(points.at(i).at(3));
        seriesMedU->append(i, newVal);
        newVals += (newVal - newVals ) * k;
        seriesKU->append(i, newVals);
        if ( points.at(i).at(3) < min) min = points.at(i).at(3);
        if ( points.at(i).at(3) > max) max = points.at(i).at(3);
    }
    chartU->removeAllSeries();
    seriesU->setName(CH1_LEG);
    seriesKU->setName(CH3_LEG);
    seriesMedU->setName(CH2_LEG);
    chartU->addSeries(seriesU);
    chartU->addSeries(seriesMedU);
    chartU->addSeries(seriesKU);
    chartU->createDefaultAxes();
    chartU->axes(Qt::Horizontal).first()->setRange(0, points.size() );
    chartU->axes(Qt::Vertical).first()->setRange((int)min, (int)max);

    if (i0.isOpen()) { i0.flush(); i0.close();}
    if (i1.isOpen()) { i1.flush(); i1.close();}
    if (i2.isOpen()) { i2.flush(); i2.close();}
    if (u.isOpen()) { u.flush(); u.close(); }


}

// медиана на 3 значений со своим буфером
float Widget::findMedianN_optim(const float &newVal) {
    if (zeroCycle) {
        shiftVec.push_back(newVal);
        shiftVec.push_back(newVal);
        shiftVec.push_back(newVal);
        zeroCycle = false;
    }
    else {
        shiftVec.remove(0);
        shiftVec.push_back(newVal);
    }
    float middle = 0;
    if ((shiftVec.at(0) <= shiftVec.at(1)) && (shiftVec.at(0) <= shiftVec.at(2)))
        middle = (shiftVec.at(1) <= shiftVec.at(2)) ? shiftVec.at(1) : shiftVec.at(2);
    else {
      if ((shiftVec.at(1) <= shiftVec.at(0)) && (shiftVec.at(1) <= shiftVec.at(2)))
          middle = (shiftVec.at(0) <= shiftVec.at(2)) ? shiftVec.at(0) : shiftVec.at(2);
      else middle = (shiftVec.at(0) <= shiftVec.at(1)) ? shiftVec.at(0) : shiftVec.at(1);
    }
    return middle;
}



void Widget::slot_cleanScreen() {
    ui->textEdit->clear();
}

Widget::~Widget() {
    if (seriesI0) delete seriesI0;
    if (seriesKI0) delete seriesKI0;
    if (seriesMedI0) delete seriesMedI0;
    delete chartI0;
    delete chartViewI0;

    if (seriesI1) delete seriesI1;
    if (seriesKI1) delete seriesKI1;
    if (seriesMedI1) delete seriesMedI1;
    delete chartI1;
    delete chartViewI1;

    if (seriesI2) delete seriesI2;
    if (seriesKI2) delete seriesKI2;
    if (seriesMedI2) delete seriesMedI2;
    delete chartI2;
    delete chartViewI2;


    if (seriesU) delete seriesU;
    if (seriesU) delete seriesKU;
    if (seriesMedU) delete seriesMedU;
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
    msgCmd.wrs.freq_msb = '!';
    msgCmd.wrs.freq_lsb = '@';
    msgCmd.wrs.end = '$';
    writeSerialPort(msgCmd, 4);
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
     msgCmd.wrs.freq_msb = '?';
     msgCmd.wrs.freq_lsb = '@';
     msgCmd.wrs.end = '$';
     writeSerialPort(msgCmd, 4);
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
