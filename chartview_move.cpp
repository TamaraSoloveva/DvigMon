/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chartview_move.h"
#include <QtGui/QMouseEvent>
#include <QDebug>

ChartView_move::ChartView_move(QChart *chart, QVector<QPointF> pV, QWidget *parent) :
    QChartView(chart, parent), ind(0), m_isTouching(false), pV(pV) {
 //   setRubberBand(QChartView::RectangleRubberBand);
}

bool ChartView_move::viewportEvent(QEvent *event) {
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;
        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView_move::mousePressEvent(QMouseEvent *event) {
    m_isTouching = true;
    if (m_isTouching) {
        auto const widgetPos = event->localPos();
        auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
        auto const chartItemPos = chart()->mapFromScene(scenePos);
        auto const valueGivenSeries = chart()->mapToValue(chartItemPos);

        int valX = qRound(valueGivenSeries.x());
        int valY = qRound(valueGivenSeries.y());
        ind = findPointInVector(valX);
        qDebug() << valueGivenSeries << " " << valX << " " << valY << " " << ind;
        for (auto p : qAsConst(pV))
            qDebug() << p;


        return;
    }
    QChartView::mousePressEvent(event);
}



int ChartView_move::findPointInVector(int x, int y) {
    int ret_ind = 0;
    float xMin = x - 1;
    float xMax = x + 1;
    for ( auto el : qAsConst(pV)) {
        if (( el.x() == x  ) || ( el.x() == xMin) || (el.x() == xMax)) {
            break;
        }
        ret_ind++;
    }
    return ret_ind;
}

void ChartView_move::mouseMoveEvent(QMouseEvent *event) {
    if (m_isTouching)
        return;
    QChartView::mouseMoveEvent(event);
}




void ChartView_move::mouseReleaseEvent(QMouseEvent *event) {
    if (m_isTouching) {
        m_isTouching = false;
        auto const widgetPos = event->localPos();
        auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
        auto const chartItemPos = chart()->mapFromScene(scenePos);
        auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
        qDebug() << valueGivenSeries << " " << ind;
        qDebug() << "point to change" << pV.at(ind);
        pV.insert(ind, QPointF(valueGivenSeries));
        pV.remove(ind+1);

        for (auto p : qAsConst(pV))
            qDebug() << p;

       emit repaintChart( pV );




    }
    // Because we disabled animations when touch event was detected
    // we must put them back on
    chart()->setAnimationOptions(QChart::SeriesAnimations);
    QChartView::mouseReleaseEvent(event);
}


//void ChartView_move::keyPressEvent(QKeyEvent *event) {
//    switch (event->key()) {
//    case Qt::Key_Plus:
//        chart()->zoomIn();
//        break;
//    case Qt::Key_Minus:
//        chart()->zoomOut();
//        break;
//    case Qt::Key_Left:
//        chart()->scroll(-10, 0);
//        break;
//    case Qt::Key_Right:
//        chart()->scroll(10, 0);
//        break;
//    case Qt::Key_Up:
//        chart()->scroll(0, 10);
//        break;
//    case Qt::Key_Down:
//        chart()->scroll(0, -10);
//        break;
//    default:
//        QGraphicsView::keyPressEvent(event);
//        break;
//    }
//}

//void ChartView_move::wheelEvent(QWheelEvent *event) {
////    qreal factor = event->angleDelta().y() > 0 ? 2.0 : 0.5;
////    chart()->zoom(factor);
////    event->accept();
////    QChartView::wheelEvent(event);

//}
