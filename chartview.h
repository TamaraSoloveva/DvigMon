#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>
#include <QWheelEvent>

QT_CHARTS_USE_NAMESPACE


class ChartView : public QChartView {
public:
    ChartView(QChart *chart, QWidget *parent = 0);

protected:
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    bool m_isTouching;
    QPoint  firstPos, lastPos;
};
