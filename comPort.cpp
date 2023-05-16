#include "comPort.h"
#include <QDebug>

SerialPort::SerialPort (QString name, qint32 spd, QSerialPort::StopBits sb,
                         QSerialPort::DataBits db, QSerialPort::Parity pp) {
    serial = new QSerialPort;
    serial->setPortName(name);
    serial->setParity(pp);
    serial->setBaudRate(spd);
    serial->setDataBits(db);
    serial->setStopBits(sb);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    try {
        serial->open(QIODevice::ReadWrite);
    }
   // catch (QSerialPort::SerialPortError &err) {
    catch (std::exception &e) {
        emit signal_outMsgWithDataCom("cxc");
    /*    if (err == QSerialPort::OpenError) emit signal_outMsgWithDataCom("Com-port is already opened");
        else if (err == QSerialPort::PermissionError) emit signal_outMsgWithDataCom("Not having enough permission to open com-port");
        else if (err == QSerialPort::DeviceNotFoundError) emit signal_outMsgWithDataCom("No com-port with this name");
        else if ( err != QSerialPort::NoError) emit signal_outMsgWithDataCom("Unable to open com-port");*/
    }


  //  connect(serial, &QSerialPort::readyRead, this, &SerialPort::readRawData );

}

SerialPort::~SerialPort() {
    serial->close();
    qDebug()<< "closed";
}

void SerialPort::readRawData() {
    QByteArray rdData = serial->readAll();
    //emit receive_data(rdData);
    emit signalSaveByteArray( rdData );

}

void SerialPort::writeRawData(QByteArray data) {
    emit sig(data);
    serial->write(data);
}

