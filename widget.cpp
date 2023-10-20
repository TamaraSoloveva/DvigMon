#include "widget.h"
#include "ui_widget.h"

const int max_dot_number = 300;
const int visible_dot_number = 30;
const int max_wait_time = 5;

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
    timerReq = nullptr;

    askMode = false;

    flMenu = new QMenu;
    actClean = new QAction("Clean screen", ui->textEdit);
    ui->textEdit->addAction(actClean);
    ui->textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->lineEdit->setValidator( new QIntValidator(1, 1000000000));
    ui->lineEdit_3->setValidator( new QIntValidator(0, 1000000000));

    //DELETE
    ui->pushButton->setEnabled(true);

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

    //ТАЙМЕРЫ
    //таймер времени работы теста
    timer = new QTimer;
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &Widget::updateTime);
    timerReq = new QTimer;
    timerReq->setInterval(500);
    connect(timerReq, &QTimer::timeout, this, &Widget::sendReq);
    //таймер запускается при отправке массива данных, если нет ответа в течение
    //max_wait_time секунд - сообщение об ошибке
    waitTimer = new QTimer();
    waitTimer->setInterval(1000);
    connect(waitTimer, &QTimer::timeout, this, &Widget::waitAnswerFromTMN);
    //таймер запроса характеристик из модуля
    askTimer = new QTimer;
    askTimer->setInterval(1000);
    ui->label_8->setText("--");
    connect(askTimer, &QTimer::timeout, this, &Widget::askInfo);




    QMenu *pmnu   = new QMenu("&Menu");
    pmnu->addAction("&Save charts", this,  SLOT(slot_saveCharts()), Qt::CTRL + Qt::Key_S);
    QAction *act = new QAction("&Use func");
    act->setCheckable(true);
    pmnu->addAction(act);
    useFuncFlag = act->isChecked();
    connect(act, &QAction::toggled, this, [&](bool bVal){useFuncFlag = bVal;});
    QAction *act2 = new QAction("&Test mode");
    act2->setCheckable(true);
    pmnu->addAction(act2);
    useTstFlag = act2->isChecked();
    connect(act2, &QAction::toggled, this, [&](bool bVal){useTstFlag = bVal;  useTstFlag ? emit signal_outMsgWithData("Test mode ON") : emit signal_outMsgWithData("Test mode OFF"); });
    act3 = new QAction("&Show coordinates");
    act3->setCheckable(true);
    act3->setChecked(true);
    pmnu->addAction(act3);
    act4 = new QAction("&Show parameters");
    act4->setCheckable(true);
    act4->setChecked(false);
    pmnu->addAction(act4);
    connect(act4, &QAction::toggled, this, &Widget::showParamsOnPanel  );
    connect(this, &Widget::reWriteLCD, this, &Widget::slot_reWriteLCD );

    QMenuBar *mnuBar;
    mnuBar = new QMenuBar();
    ui->gridLayout_3->addWidget(mnuBar);
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

    manualAdjMode();
}

void Widget::askInfo() {
    //раз в секунду запрос в ТМН
    msgCmd.wrs.strt = '#';
    msgCmd.wrs.freq_msb = 0;
    msgCmd.wrs.freq_lsb = 0;
    msgCmd.wrs.end = '!';
    writeSerialPort(msgCmd);
}

void Widget::showParamsOnPanel() {
    if (act4->isChecked()) {
        askMode = true;
        askTimer->start();
    }
    else {
        askTimer->stop();
        askMode = false;
    }
}

/* Функция для получения рандомного числа
 * в диапазоне от минимального до максимального
 * */
int Widget::randomBetween(const int &low, const int &high) {
    QRandomGenerator generator;
    return (generator.generate() % ((high + 1) - low) + low);
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

void Widget::slot_reWriteLCD(const quint16 &i, const quint16 &f, const char &mode) {
    QString modeS;
    if (mode == 0x5)
        modeS = "ожидание";
    else if (mode == 0x8)
            modeS = "разгон";
    else if (mode == 0x10)
            modeS = "торможение";
    else if (mode == 0x15)
            modeS = "рабочий режим";
    else modeS = "ошибка приема";
    QString res = "ток: " + QString::number((float)i/1000)  + QString(" A   частота: ")+QString::number(f) + QString(" Hz  режим: ")+ modeS;
    ui->label_8->clear();
    ui->label_8->setText(res);
}


void Widget::readRawData() {
    QByteArray rdData = m_serial->readAll();
//    rdData.resize(6);
//    rdData[0] = '@';
//    rdData[1] = 0x8B;
//    rdData[2] = 0xB;
//    rdData[0] = '@';
//    rdData[0] = '@';
//    rdData[0] = '@';
    if (askMode) {
        if (rdData.size() >= 6) {
            //infoBuf.push_back(rdData);
            quint16 i = static_cast<quint8>(rdData.at(1)) | rdData.at(2) << 8;
            quint16 f = static_cast<quint8>(rdData.at(3)) | (rdData.at(4) << 8 );
            unsigned char mode = rdData.at(5);
            emit reWriteLCD(i, f, mode) ;
            rdData.remove(0, 6);
        }
    }
    if (rdData.size() >= 4) {
        if ((rdData.at(0) == '@') && (rdData.at(1) == 0x41) && (rdData.at(2) == 0x53) && (rdData.at(3) == 0x53) ) {
            rdData.remove(0, 4);
            QMessageBox::warning(this, "Error", "CRC error", QMessageBox::Ok);
            waitTimer->stop();
        }
        else if ((rdData.at(0) == '@') && (rdData.at(1) == 0x0) && (rdData.at(2) == 0x4f) && (rdData.at(3) == 0x4B) ) {
            rdData.remove(0, 4);
            QMessageBox::information(this, "Info", "Packet received", QMessageBox::Ok);
            waitTimer->stop();
        }
        else if ((rdData.at(0) == '@') && (rdData.at(1) == 0x50) && (rdData.at(2) == 0x49) && (rdData.at(3) == 0x47) ) {
            rdData.remove(0, 4);
            QMessageBox::warning(this, "Error", "Packet received ERROR", QMessageBox::Ok);
            waitTimer->stop();
        }
    }
    if (rdData.size())
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


void Widget::manualAdjMode() {
    chart = new QChart;
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTitle("Equaliser");

    sLine = new QLineSeries;
    sLine->setColor(Qt::blue);
    sDots = new QScatterSeries;
    sDots->setMarkerSize(8);
    sDots->setColor(Qt::blue);

    pVec.clear();
    for (int x = 0; x <= max_dot_number; x+= max_dot_number/visible_dot_number) {
        float y = getChartValue(QPointF(0, 0), QPointF(max_dot_number, 100), x);
        sDots->append(QPointF(x, y));
        sLine->append(QPointF(x, y));
        pVec.push_back(QPointF((float)x, y));
    }

    chart->addSeries(sDots);
    chart->addSeries(sLine);
    chart->legend()->hide();

    chView = new ChartView_move(chart, max_dot_number, pVec);
    connect(act3, &QAction::toggled, chView, [&](bool ch) { ui->label_7->setVisible(ch); });
    connect(chView, &ChartView_move::showInfoOnLabel, this, [&](QPointF point) {
        if (ui->label_7->isVisible() ) ui->label_7->setText(QString::number(point.x()) + ", " + QString::number(point.y()));});
    ui->gridLayout->addWidget(chView, 0, 0);

    auto axisX = new QValueAxis;
    auto axisY = new QValueAxis;
    axisX->setRange(0, max_dot_number);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Frequency, Hz");
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%g");
    axisY->setTitleText("Amplitude, %");

    chart->addAxis(axisX, Qt::AlignBottom);
    sDots->attachAxis(axisX);
    chart->addAxis(axisY, Qt::AlignLeft);
    sDots->attachAxis(axisY);
    sLine->attachAxis(axisX);
    sLine->attachAxis(axisY);

    connect(chView, &ChartView_move::repaintChart, this, &Widget::slot_repaintChart);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &Widget::resetChart);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &Widget::writeVecToCom);
    connect(this, &Widget::signal_resetVec, chView, &ChartView_move::resetVector);
    connect(ui->pushButton_8, &QPushButton::clicked, this, &Widget::openChart);
    connect(ui->pushButton_9, &QPushButton::clicked, this, &Widget::saveChart);
}

void Widget::waitAnswerFromTMN() {
    sec_num_wait++;
    if (sec_num_wait == max_wait_time) {
        waitTimer->stop();
        QMessageBox::critical(this, "Error", "No answer received!", QMessageBox::Ok);
    }

}

QVector<QPointF> Widget::countAmpl() {
    QVector<QPointF> amplVec;
    float amp = 0.;
    for (int x = 0; x <= max_dot_number; ++x ) {
        for (int i = 0; i < pVec.size()-1; ++i) {
            if ((x >= pVec.at(i).x()) && (x < pVec.at(i+1).x()) ) {
                amp = getChartValue(pVec.at(i), pVec.at(i+1), static_cast<float>(x));
                amplVec.push_back(QPointF(x, amp));
                break;
             }
            else {
                continue;
           }
        }
    }
    return amplVec;
}

void Widget::writeVecToCom( ) {
    const QVector<QPointF> &ampl = countAmpl();
    if (!m_serial->isOpen()) {
        ui->label_7->setText("NO CONNECTION!!!");
        QMessageBox::warning(this, "Error!", "No connection to COM port", QMessageBox::Ok );
    }
    else {
        sec_num_wait = 0;
        waitTimer->start();
        QFile fl_aTmp("send.txt");
        if (!fl_aTmp.open(QIODevice::WriteOnly) ) {
           QMessageBox::critical(nullptr, "Error", "Unable to open file", QMessageBox::Ok);
        }
        wrCmdMsg msgCmd;
        chSm = 0;
        unsigned char packNum = 0;
        QVector<QPointF>::const_iterator it;
        for ( it = ampl.begin(); it < ampl.end();  ) {
            if (ui->radioButton->isChecked())
                msgCmd.wrs.strt = '%';
            else
                msgCmd.wrs.strt = '&';

            msgCmd.wrs.freq_msb = packNum;
            QString sss;

            msgCmd.wrs.freq_lsb = qRound(it->y());
            sss = QString("%1 - %2\n").arg(QString::number(packNum)).arg(it->y());
            fl_aTmp.write(sss.toUtf8());
            chSm ^= qRound(it->y());
            it++;

            msgCmd.wrs.end = qRound(it->y());
            sss = QString("%1 - %2\n").arg(QString::number(packNum)).arg(it->y());
            fl_aTmp.write(sss.toUtf8());
            chSm ^= qRound(it->y());

            packNum++;
            it++;

            writeSerialPort(msgCmd);
        }
        if (ui->radioButton->isChecked())
            msgCmd.wrs.strt = '%';
        else
            msgCmd.wrs.strt = '&';
        int tmp = chSm;
        chSm &= 0xFF;
        msgCmd.wrs.freq_msb = chSm;
        chSm = tmp;
        chSm >>= 8;
        chSm &= 0xFF;
        msgCmd.wrs.freq_lsb = chSm;
        chSm = tmp;
        chSm >>= 16;
        chSm &= 0xFF;
        msgCmd.wrs.end = chSm;
        writeSerialPort(msgCmd);
        QString sss = QString("ChSm: %1 %2 %3").arg(msgCmd.wrs.freq_msb).arg(msgCmd.wrs.freq_lsb).arg(msgCmd.wrs.end);
        fl_aTmp.write(sss.toUtf8());
        fl_aTmp.flush();
        fl_aTmp.close();
    }
}

void Widget::resetChart( ) {
    sDots->clear();
    sLine->clear();
    pVec.clear();
    for (int x = 0; x <= max_dot_number; x+= max_dot_number/visible_dot_number) {
        float y = getChartValue(QPointF(0, 0), QPointF(300, 100), x);
        sDots->append(QPointF(x, y));
        sLine->append(QPointF(x, y));
        pVec.push_back(QPointF((float)x, y));
    }
    emit signal_resetVec(pVec);
}

float Widget::getChartValue(const QPointF &p1, const QPointF &p2, const float &x) {
    float y = ((x - p1.x())*(p2.y() - p1.y()))/((p2.x() - p1.x())) + p1.y();
    return y;
}


void Widget::slot_repaintChart( const QVector<QPointF> &vect ) {
    sDots->clear();
    sLine->clear();
    for (auto x : vect) {
        *sDots << x;
        *sLine << x;
    }
    pVec = vect;
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
    if (useFuncFlag)
        return ((float) v * 0.076 - 2500);
    else
       return(float)v;
}

void Widget::slot_ParseResult() {
    float k = 0;
    if (!ui->lineEdit_4->text().isEmpty()) {
        k = ui->lineEdit_4->text().toFloat();
        if (k < 0 || k > 1 ) {
            emit signal_outMsgWithData("k value - error");
            return;
        }
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
    if (k) {
        seriesKI0 = new QLineSeries();
        seriesMedI0 = new QLineSeries();
    }

    min = points.at(0).at(0);
    max = points.at(0).at(0);
    float j = 0.5;
    for (int i=0; i<sz; ++i){
        if (i0.isOpen()) {
            i0.write( QByteArray::number(points.at(i).at(0)));
            i0.write( " " );
        }
        if (useTstFlag) {
//              QValueAxis *axisX = new QValueAxis();
//              axisX->setMin(1);
//              axisX->setTickCount(500);

            seriesI0->append(j, points.at(i).at(0));
            j += 0.5;
        }
        else
            seriesI0->append(i, points.at(i).at(0));
        if (k) {
            newVal = findMedianN_optim(points.at(i).at(0));
            seriesMedI0->append(i, newVal);
            newVals += (newVal - newVals ) * k;
            seriesKI0->append(i, newVals);
        }
        if ( points.at(i).at(0) < min) min = points.at(i).at(0);
        if ( points.at(i).at(0) > max) max = points.at(i).at(0);
    }
    chartI0->removeAllSeries();
    seriesI0->setName(CH1_LEG);
    chartI0->addSeries(seriesI0);
    if (k) {
        seriesMedI0->setName(CH2_LEG);
        seriesKI0->setName(CH3_LEG);
        chartI0->addSeries(seriesMedI0);
        chartI0->addSeries(seriesKI0);
    }
    chartI0->createDefaultAxes();
 //i1
    if (useTstFlag)
        return;
    if (seriesI1) delete seriesI1;
    if (seriesKI1) delete seriesKI1;
    if (seriesMedI1) delete seriesMedI1;
    seriesI1 = new QLineSeries();
    if (k) {
        seriesKI1 = new QLineSeries();
        seriesMedI1 = new QLineSeries();
        newVals = 0;
        newVal = 0;
    }
    min = points.at(0).at(1);
    max = points.at(0).at(1);

    for (int i=0; i<sz; ++i){
        if (i1.isOpen()) {
            i1.write( QByteArray::number(points.at(i).at(1)));
            i1.write( " " );
        }
         seriesI1->append(i, points.at(i).at(1));
         if (k) {
            newVal = findMedianN_optim( points.at(i).at(1));
            seriesMedI1->append(i, newVal);
            newVals += (newVal - newVals ) * k;
            seriesKI1->append(i, newVals);
         }
         if ( points.at(i).at(1) < min) min = points.at(i).at(1);
         if ( points.at(i).at(1) > max) max = points.at(i).at(1);
     }
    chartI1->removeAllSeries();
    seriesI1->setName(CH1_LEG);
    chartI1->addSeries(seriesI1);
    if (k) {
        seriesMedI1->setName(CH2_LEG);
        seriesKI1->setName(CH3_LEG);
        chartI1->addSeries(seriesMedI1);
        chartI1->addSeries(seriesKI1);
    }
    chartI1->createDefaultAxes();
//i2
    if (seriesI2) delete seriesI2;
    if (seriesKI2) delete seriesKI2;
    if (seriesMedI2) delete seriesMedI2;
    seriesI2 = new QLineSeries();
    if(k) {
        seriesKI2= new QLineSeries();
        seriesMedI2 = new QLineSeries();
        newVal = 0;
        newVals = 0;
    }
    min = points.at(0).at(2);
    max = points.at(0).at(2);
    for (int i=0; i<sz; ++i){
        if (i2.isOpen()) {
            i2.write( QByteArray::number(points.at(i).at(2)));
            i2.write( " " );
        }
        seriesI2->append(i, points.at(i).at(2));
        if (k) {
            newVal = findMedianN_optim(points.at(i).at(2));
            seriesMedI2->append(i, newVal);
            newVals += (newVal - newVals ) * k;
            seriesKI2->append(i, newVals);
        }
        if ( points.at(i).at(2) < min) min = points.at(i).at(2);
        if ( points.at(i).at(2) > max) max = points.at(i).at(2);
    }
    chartI2->removeAllSeries();
    seriesI2->setName(CH1_LEG);
    chartI2->addSeries(seriesI2);
    if (k) {
        seriesKI2->setName(CH3_LEG);
        seriesMedI2->setName(CH2_LEG);
        chartI2->addSeries(seriesMedI2);
        chartI2->addSeries(seriesKI2);
    }
    chartI2->createDefaultAxes();
//u
    if (seriesU) delete seriesU;
    if (seriesKU) delete seriesKU;
    if (seriesMedU) delete seriesMedU;
    seriesU = new QLineSeries();
    if (k) {
        seriesKU = new QLineSeries();
        seriesMedU = new QLineSeries();
        newVal = 0;
        newVals = 0;
    }
    min = points.at(0).at(3);
    max = points.at(0).at(3);
    for (int i=0; i<sz; ++i){
        if (u.isOpen()) {
            u.write( QByteArray::number(points.at(i).at(3)));
            u.write( " " );
        }
        seriesU->append(i, points.at(i).at(3));
        if (k) {
            newVal = findMedianN_optim(points.at(i).at(3));
            seriesMedU->append(i, newVal);
            newVals += (newVal - newVals ) * k;
            seriesKU->append(i, newVals);
        }
        if ( points.at(i).at(3) < min) min = points.at(i).at(3);
        if ( points.at(i).at(3) > max) max = points.at(i).at(3);
    }
    chartU->removeAllSeries();
    seriesU->setName(CH1_LEG);
    chartU->addSeries(seriesU);
    if (k) {
        seriesKU->setName(CH3_LEG);
        seriesMedU->setName(CH2_LEG);
        chartU->addSeries(seriesMedU);
        chartU->addSeries(seriesKU);
    }
    chartU->createDefaultAxes();

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

    if (askTimer->isActive())
        askTimer->stop();
    delete askTimer;

    slot_stopConnection();

    delete timer;
    delete timerReq;
    delete waitTimer;
    delete chartU;
    delete chartViewU;


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
    stopTest(true);
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
    timer->start();

    QString aa;
    if (useTstFlag) {
        timerReq->start();
        aa = QDir::currentPath()+"\\DATA_test.txt";
    }
    else {
       aa = QDir::currentPath()+"\\DATA.txt";
    }

    fl.setFileName(aa);
    if (fl.open(QIODevice::WriteOnly)) {
        emit signal_outMsgWithData("log file " + aa + " opened");
    }
    else {
        emit signal_outMsgWithData("Unable to open log file ");
    }

    if (!useTstFlag) {
        msgCmd.wrs.strt = '@';
        msgCmd.wrs.freq_msb = '!';
        msgCmd.wrs.freq_lsb = '@';
        msgCmd.wrs.end = '$';
        writeSerialPort(msgCmd, 4);
    }
}


void Widget::writeSerialPort( wrCmdMsg & msgCmd, size_t sz ) {
    if (!sz) sz = sizeof(msgCmd.msgMas);
    QByteArray data = QByteArray(reinterpret_cast< char *>(msgCmd.msgMas), sz);
    if (m_serial->isOpen()) m_serial->write(data);
}

void Widget::stopTest(bool byBtn) {
    if (timer && timer->isActive()) {
        timer->stop();
    }
    if (timerReq && timerReq->isActive())
        timerReq->stop();

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

     if (!useTstFlag) {
         msgCmd.wrs.strt = '@';
         msgCmd.wrs.freq_msb = '?';
         msgCmd.wrs.freq_lsb = '@';
         msgCmd.wrs.end = '$';
         writeSerialPort(msgCmd, 4);
     }
}

void Widget::sendReq() {
    msgCmd.wrs.strt = '@';
    msgCmd.wrs.freq_msb = '0';
    msgCmd.wrs.freq_lsb = 0;
    msgCmd.wrs.end ='+';
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

void Widget::openChart() {
    QString filename = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open chart"), QDir::currentPath(), QObject::tr("Coordinates files (*.txt);;All files (*.*)"));
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
           return;
    pVec.clear();
    bool dot = false;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str = QString::fromUtf8(line).simplified();
        if (str == "*") {
            dot = true;
            continue;
        }
        if (dot) {
            QStringList strList = str.split(QRegExp(","));
            QVector<float>tmp;
            for (const auto &s : qAsConst(strList)) {
                tmp.push_back( s.simplified().toFloat()) ;
            }
            pVec.push_back(QPointF(tmp.at(0), tmp.at(1)));
        }
    }
    emit signal_resetVec(pVec);
    slot_repaintChart(pVec);
    file.close();
}


void Widget::saveChart() {
    QVector<QPointF> amplVec = countAmpl();
    QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save chart"), QDir::currentPath(), QObject::tr("Coordinates files (*.txt);;All files (*.*)"));
    if (filename.isEmpty()) return;
    QFile flAmpl(filename);
    if (!flAmpl.open(QIODevice::WriteOnly) ) {
       QMessageBox::critical(nullptr, "Error", "Unable to open file", QMessageBox::Ok);
       return;
    }
    QVector<QPointF>::iterator it;
    for ( it = amplVec.begin(); it < amplVec.end(); ++it  ) {
        QString sss = QString("%1, %2\n").arg(it->x()).arg(it->y());
        flAmpl.write(sss.toUtf8());
    }
    flAmpl.write("*\n");
    for ( it = pVec.begin(); it < pVec.end(); ++it  ) {
        QString sss = QString("%1, %2\n").arg(it->x()).arg(it->y());
        flAmpl.write(sss.toUtf8());
    }
    flAmpl.flush();
    flAmpl.close();
    //CHECK
//    QScatterSeries *www = new QScatterSeries;
//    www->setMarkerSize(5);
//    www->setColor(Qt::red);
//    for (auto x : amplVec) {
//        www->append(QPoint(x.x(), x.y()));

//    }
//    chart->addSeries(www);

}
