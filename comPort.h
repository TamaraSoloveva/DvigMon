#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <QDebug>
#include <QtSerialPort/QSerialPort>


class SerialPort : public QObject
{
    Q_OBJECT
public:
    SerialPort ( QString name, qint32 spd, QSerialPort::StopBits sb,
                 QSerialPort::DataBits db, QSerialPort::Parity pp );
    ~SerialPort();

private slots:
    void readRawData();



private:
    QSerialPort *serial;
    void writeRawData(QByteArray data );

public slots:
    void slot_writeData(QByteArray data) {  qDebug()<<"data1="<<data.size(); writeRawData(data); }

signals:
    void receive_data(QByteArray tmp);
    void signalSaveByteArray(QByteArray tmp);
    void sig(QByteArray aaa);


};

#endif // COMPORT_H
