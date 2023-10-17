#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QWheelEvent>
QT_CHARTS_USE_NAMESPACE


class ChartView_move : public QChartView {
    Q_OBJECT
public:
    ChartView_move(QChart *chart, const int &max_dot_number, QVector<QPointF> pV, QWidget *parent = 0);

protected:
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int findPointInVector(float x, float y);
    int ind;
    bool m_isTouching;
    int max_dot_nmb;
    QVector<QPointF> pV;
signals:
    void repaintChart( const QVector<QPointF> & pV);
    void showInfoOnLabel(const QPointF &point);
public slots:
    void resetVector(const QVector<QPointF> & vInput);
};
