#include "widget.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  
    Widget w;
 //   w.setWindowIcon(QIcon(":/new/prefix1/free-icon-monitor-3286914.ico"));

//    QLabel lbl("sssss");
//    lbl.show();

    w.show();
    return a.exec();
}
