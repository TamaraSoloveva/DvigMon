#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QWheelEvent>

QT_CHARTS_USE_NAMESPACE


class ChartView_move : public QChartView {
    Q_OBJECT
public:
    ChartView_move(QChart *chart, QVector<QPointF> pV, bool bShow, QWidget *parent = 0);
    bool b_showCoordinates;

protected:
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
//    void keyPressEvent(QKeyEvent *event) override;
//    void wheelEvent(QWheelEvent *event) override;

private:
    int findPointInVector(float x, float y);
    int ind;
    bool m_isTouching;

    QVector<QPointF> pV;
signals:
    void repaintChart( const QVector<QPointF> & pV);
    void showCoorinates(const QPointF &point);
public slots:
    void resetVector(const QVector<QPointF> & vInput) {pV.clear(); pV = vInput; }
};
