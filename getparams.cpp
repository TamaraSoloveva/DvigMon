#include "getparams.h"


getParams::getParams(QWidget *parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint ) {
        QLabel *lbl1x = new QLabel("X: min");
        QLabel *lbl2x = new QLabel("X: max");
        QLabel *lbl1y = new QLabel("Y: min");
        QLabel *lbl2y = new QLabel("Y: max");
        ed1x = new QLineEdit;
        ed2x = new QLineEdit;
        ed1y = new QLineEdit;
        ed2y = new QLineEdit;
        QGridLayout *lay = new QGridLayout;
        QPushButton *btn = new QPushButton("&OK");
        QPushButton *btn2 = new QPushButton("&Cancel");

        connect(btn, &QPushButton::clicked, this, &getParams::accept );
        connect(btn2, &QPushButton::clicked, this,  &getParams::reject );

        lay->addWidget(lbl1x, 0, 0);
        lay->addWidget(ed1x, 0, 1);
        lay->addWidget(lbl2x, 0, 2);
        lay->addWidget(ed2x, 0, 3);

        lay->addWidget(lbl1y, 1, 0);
        lay->addWidget(ed1y, 1, 1);
        lay->addWidget(lbl2y, 1, 2);
        lay->addWidget(ed2y, 1, 3);

        lay->addWidget(btn, 2, 1);
        lay->addWidget(btn2, 2, 3);
        setLayout(lay);
}


getParams::~getParams()
{

}
