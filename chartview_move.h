#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QWheelEvent>

QT_CHARTS_USE_NAMESPACE


class ChartView_move : public QChartView {
    Q_OBJECT
public:
    ChartView_move(QChart *chart, QVector<QPointF> pV, QWidget *parent = 0);

protected:
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
//    void keyPressEvent(QKeyEvent *event) override;
//    void wheelEvent(QWheelEvent *event) override;

private:
    int findPointInVector(int x, int y = 0);
    int ind;
    bool m_isTouching;
//  QPoint  firstPos, lastPos;
    QVector<QPointF> pV;
signals:
    void repaintChart( const QVector<QPointF> & pV);
};
