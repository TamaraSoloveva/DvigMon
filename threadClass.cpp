#include "threadClass.h"
#include <QDebug>
#include <sstream>


void threadClass::run()  {
    qDebug()<<"Started thread\n";

    std::ostringstream ss;
     std::string s;

    while(started) {
        if (!dataVec.isEmpty()) {
            for (numInArr=0; numInArr<dataVec.size(); ++numInArr ) {
                if((dataVec.at(numInArr) != '@') && (cntr == 0))
                    continue;

                switch(cntr) {
                case 1:
                case 3:
                case 5:
                case 7:
                case 9:
                    val = dataVec.at(numInArr);
                    break;
                case 2:
                case 4:
                case 6:
                case 8:
                case 10:
                    tmp = dataVec.at(numInArr);
                    tmp <<= 8;
                    val += tmp;
                    params.push_back(val);
                    ss << std::hex << val;
                    s = ss.str() + " ";
                    fl_tmp.write(s.c_str());
                    s.clear();
                    ss.str("");
                    ss.clear();
                    val = 0;
                    break;
                case 11:
                    if (dataVec.at(numInArr) == '!') {
                        points.push_back(params);
                        params.clear();
                    }
                    else {
                      //  emit signal_outMsgWithData("Bad pack");

                    }
                    params.clear();
                    cntr=0;
                    continue;
                }
                cntr++;
            }
            dataVec.clear();
        }
    }
    qDebug()<<"stopped";

}
