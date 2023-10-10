#ifndef GETPARAMS_H
#define GETPARAMS_H

#include <QDialog>
#include <QSharedDataPointer>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>

class getParamsData;

class getParams : public QDialog
{
    Q_OBJECT
public:
    getParams( QWidget *parent = 0);
    ~getParams();
    double minX() const { return ed1x->text().isEmpty() ? 0 : ed1x->text().toDouble(); }
    double maxX() const { return ed2x->text().isEmpty() ? 0 : ed2x->text().toDouble(); }
    double minY() const { return ed1y->text().isEmpty() ? 0 : ed1y->text().toDouble(); }
    double maxY() const { return ed2y->text().isEmpty() ? 0 : ed2y->text().toDouble(); }

private:
    QLineEdit *ed1x;
    QLineEdit *ed2x;
    QLineEdit *ed1y;
    QLineEdit *ed2y;


};

#endif // GETPARAMS_H
