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

ChartView_move::ChartView_move(QChart *chart, QVector<QPointF> pV, bool bShow, QWidget *parent) :
    QChartView(chart, parent), b_showCoordinates(bShow), ind(0), m_isTouching(false), pV(pV) {
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
        ind = findPointInVector(valueGivenSeries.x(), valueGivenSeries.y());
        if (ind == -1)
             m_isTouching = false;
        return;
    }
    QChartView::mousePressEvent(event);
}


int ChartView_move::findPointInVector(float x, float y) {
    int ret_ind = 0;
    bool pointDetected = false;

    qDebug() << "++++++++++++++++++++++++++++++\n" << x << " " << y;
    for ( auto el : qAsConst(pV)) {
        qDebug() << el.x() << " " << el.y();
        if (( el.x() - 2) <= x && x < (el.x() + 2) ) {
            if (( el.y() - 1) <= y && y < (el.y() + 1)) {
                pointDetected = true;
                break;
            }
            else {
                ret_ind++;
                continue;
            }
        }
 //      qDebug() << el.x() - 0.2 << " " << el.x() + 0.2;
//       qDebug() << el.y() - 0.2 << " " << el.y() + 0.2;
        ret_ind++;
    }
    if (!pointDetected)
        ret_ind = -1;

    qDebug() << "ret_ind " << ret_ind;

    return ret_ind;
}

void ChartView_move::mouseMoveEvent(QMouseEvent *event) {
    if (m_isTouching)
        return;
    else {
        auto const widgetPos = event->localPos();
        auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
        auto const chartItemPos = chart()->mapFromScene(scenePos);
        auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
        if (b_showCoordinates)
            emit showCoorinates(valueGivenSeries);
    }
    QChartView::mouseMoveEvent(event);
}




void ChartView_move::mouseReleaseEvent(QMouseEvent *event) {
    if (m_isTouching) {
        m_isTouching = false;
        if (ind == -1) return;
        auto const widgetPos = event->localPos();
        auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
        auto const chartItemPos = chart()->mapFromScene(scenePos);
        auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
   //     qDebug() << valueGivenSeries;
 //       qDebug() << "point to change" << pV.at(ind);
        pV.insert(ind, QPointF(valueGivenSeries));
        pV.remove(ind+1);

//        for (auto p : qAsConst(pV))
//            qDebug() << p;

       emit repaintChart( pV );




    }
    // Because we disabled animations when touch event was detected
    // we must put them back on
    chart()->setAnimationOptions(QChart::SeriesAnimations);
    QChartView::mouseReleaseEvent(event);
}


