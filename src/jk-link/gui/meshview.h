// jk-link gui's topology view: this machine at the centre, the machines it
// knows around it, laid out by a small force simulation, with the links
// between them (its own, and those its paired machines report), their
// round-trip times and the traffic on them. QPainter, with pan, zoom,
// select and drag.
#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QPointF>
#include <QQuickPaintedItem>
#include <QTimer>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class MeshView : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QVariantMap status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool showOffline READ showOffline WRITE setShowOffline NOTIFY filterChanged)
    Q_PROPERTY(bool showUnpaired READ showUnpaired WRITE setShowUnpaired NOTIFY filterChanged)
    Q_PROPERTY(double zoom READ zoom NOTIFY viewChanged)
    // Space fit() leaves free at the top and bottom (the toolbar, the legend).
    Q_PROPERTY(double fitTop MEMBER fitTop_)
    Q_PROPERTY(double fitBottom MEMBER fitBottom_)

public:
    explicit MeshView(QQuickItem *parent = nullptr);

    QVariantMap status() const { return status_; }
    void setStatus(const QVariantMap &s);
    QString selected() const { return selected_; }
    void setSelected(const QString &key);
    bool showOffline() const { return showOffline_; }
    void setShowOffline(bool v);
    bool showUnpaired() const { return showUnpaired_; }
    void setShowUnpaired(bool v);
    double zoom() const { return zoom_; }

    void paint(QPainter *p) override;

    Q_INVOKABLE void fit();
    Q_INVOKABLE void zoomBy(double factor);
    Q_INVOKABLE void relayout();

signals:
    void statusChanged();
    void selectedChanged();
    void filterChanged();
    void viewChanged();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void hoverMoveEvent(QHoverEvent *e) override;
    void hoverLeaveEvent(QHoverEvent *e) override;

private:
    enum class Kind { Self, Paired, Seen, Remote };
    struct Node {
        QString key, id, name, arch;
        Kind kind = Kind::Seen;
        bool online = false, request = false, pairing = false, renamed = false;
        QString link;     // up, connecting, down
        double rtt = -1;
        QPointF pos, vel;
        bool pinned = false;
        bool present = true;
    };
    enum class EdgeKind { Link, Connecting, Discovery, Remote };
    struct Edge {
        QString a, b;
        EdgeKind kind;
        double rtt = -1;
        double phase = 0;
    };

    void rebuild();
    void step(double dt);
    QPointF toScreen(QPointF w) const;
    QPointF toWorld(QPointF s) const;
    QString nodeAt(QPointF screen) const;
    double radius(const Node &n) const;

    void drawBackground(QPainter *p);
    void drawEdge(QPainter *p, const Edge &e);
    void drawNode(QPainter *p, const Node &n);

    QVariantMap status_;
    QHash<QString, Node> nodes_;
    QList<Edge> edges_;
    QString selfKey_, selected_, hover_, dragging_;
    bool panning_ = false, moved_ = false;
    QPointF pan_, pressAt_, lastMouse_;
    double zoom_ = 1.0;
    double fitTop_ = 0, fitBottom_ = 0;
    bool showOffline_ = true, showUnpaired_ = true;
    bool autoFit_ = true;
    double sweep_ = 0;    // the discovery sweep around this machine, radians
    double clock_ = 0;    // seconds, for blinking
    QTimer tick_;
    QElapsedTimer frame_;
};
