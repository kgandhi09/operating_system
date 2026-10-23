// jk-viz: draws the graph (QPainter), with pan, zoom, select and collapse.
#pragma once
#include <QPointF>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

#include "graphmodel.h"

class GraphView : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(GraphModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(double zoom READ zoom NOTIFY viewChanged)

public:
    explicit GraphView(QQuickItem *parent = nullptr);
    GraphModel *model() const { return model_; }
    void setModel(GraphModel *m);
    double zoom() const { return zoom_; }
    void paint(QPainter *p) override;

    Q_INVOKABLE void fit();
    Q_INVOKABLE void home();
    Q_INVOKABLE void centerOnNode(const QString &id);
    Q_INVOKABLE void zoomBy(double f);

signals:
    void modelChanged();
    void viewChanged();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void hoverMoveEvent(QHoverEvent *e) override;

private:
    GraphModel *model_ = nullptr;
    double zoom_ = 0.6;
    QPointF pan_{20, 20};       // scene origin on screen
    QPointF press_, panAtPress_;
    bool dragging_ = false, placed_ = false;
    int hover_ = -1;
    int hit(QPointF pos) const;
    QPointF toScene(QPointF p) const { return (p - pan_) / zoom_; }
};
