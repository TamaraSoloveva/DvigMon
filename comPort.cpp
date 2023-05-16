#include "comPort.h"
#include <QDebug>
#include <QMessageBox>
#include <QErrorMessage>

SerialPort::SerialPort (QString name, qint32 spd, QSerialPort::StopBits sb,
                         QSerialPort::DataBits db, QSerialPort::Parity pp) {
    serial = new QSerialPort;
    serial->setPortName(name);
    serial->setParity(pp);
    serial->setBaudRate(spd);
    serial->setDataBits(db);
    serial->setStopBits(sb);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(nullptr, tr("Error"), serial->errorString());
    }

    connect(serial, &QSerialPort::readyRead, this, &SerialPort::readRawData );

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

