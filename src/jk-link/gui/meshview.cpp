#include "meshview.h"

#include <QConicalGradient>
#include <QCursor>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QtMath>
#include <cmath>

namespace {

// The console's palette (also in Main.qml).
const QColor kBg(0x07, 0x0b, 0x10);
const QColor kGridMinor(0x0e, 0x15, 0x1d);
const QColor kGridMajor(0x15, 0x1f, 0x2a);
const QColor kTeal(0x1f, 0xd1, 0xc1);
const QColor kAmber(0xf5, 0xa5, 0x24);
const QColor kBlue(0x4c, 0x8d, 0xff);
const QColor kSlate(0x4a, 0x55, 0x62);
const QColor kRed(0xff, 0x4d, 0x5e);
const QColor kText(0xe6, 0xed, 0xf3);
const QColor kDim(0x7d, 0x8a, 0x99);
const QColor kPanel(0x0b, 0x12, 0x19);

QColor alpha(QColor c, int a)
{
    c.setAlpha(a);
    return c;
}

QPainterPath hexagon(QPointF c, double r)
{
    QPainterPath path;
    for (int i = 0; i < 6; ++i) {
        double a = qDegreesToRadians(60.0 * i - 90);
        QPointF p = c + QPointF(r * std::cos(a), r * std::sin(a));
        if (i == 0)
            path.moveTo(p);
        else
            path.lineTo(p);
    }
    path.closeSubpath();
    return path;
}

QString rttText(double rtt)
{
    if (rtt < 0)
        return QStringLiteral("— ms");
    if (rtt < 1)
        return QString::number(rtt, 'f', 2) + QStringLiteral(" ms");
    if (rtt < 10)
        return QString::number(rtt, 'f', 1) + QStringLiteral(" ms");
    return QString::number(qRound(rtt)) + QStringLiteral(" ms");
}

QFont uiFont(double px, int weight = QFont::Normal)
{
    QFont f(QStringLiteral("Inter"));
    f.setPixelSize(qMax(6, int(px)));
    f.setWeight(QFont::Weight(weight));
    return f;
}

QFont monoFont(double px)
{
    QFont f(QStringLiteral("FiraCode Nerd Font Mono"));
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(qMax(6, int(px)));
    return f;
}

} // namespace

MeshView::MeshView(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
    setFillColor(kBg);
    tick_.setInterval(33);
    connect(&tick_, &QTimer::timeout, this, [this] {
        double dt = frame_.isValid() ? frame_.restart() / 1000.0 : 0.033;
        dt = qMin(dt, 0.1);
        step(dt);
        update();
    });
    tick_.start();
    frame_.start();
}

void MeshView::setStatus(const QVariantMap &s)
{
    status_ = s;
    rebuild();
    emit statusChanged();
}

void MeshView::setSelected(const QString &key)
{
    if (key == selected_)
        return;
    selected_ = key;
    emit selectedChanged();
    update();
}

void MeshView::setShowOffline(bool v)
{
    if (v == showOffline_)
        return;
    showOffline_ = v;
    rebuild();
    emit filterChanged();
}

void MeshView::setShowUnpaired(bool v)
{
    if (v == showUnpaired_)
        return;
    showUnpaired_ = v;
    rebuild();
    emit filterChanged();
}

double MeshView::radius(const Node &n) const
{
    switch (n.kind) {
    case Kind::Self: return 30;
    case Kind::Paired: return 22;
    case Kind::Seen: return 18;
    case Kind::Remote: return 12;
    }
    return 18;
}

// The graph from jk-linkd's status: nodes keep their place across updates.
void MeshView::rebuild()
{
    const QVariantMap self = status_.value(QStringLiteral("self")).toMap();
    selfKey_ = self.value(QStringLiteral("key")).toString();
    if (selfKey_.isEmpty())
        return;
    for (auto &n : nodes_)
        n.present = false;

    QSet<QString> requests, pairings;
    for (const QVariant &v : status_.value(QStringLiteral("requests")).toList())
        requests.insert(v.toMap().value(QStringLiteral("key")).toString());
    for (const QVariant &v : status_.value(QStringLiteral("pairings")).toList()) {
        QVariantMap m = v.toMap();
        QString st = m.value(QStringLiteral("state")).toString();
        if (st == QLatin1String("connecting") || st == QLatin1String("waiting"))
            pairings.insert(m.value(QStringLiteral("key")).toString());
    }

    auto place = [&](Node &n, const QString &near) {
        QPointF base = nodes_.contains(near) ? nodes_[near].pos : QPointF();
        double a = QRandomGenerator::global()->bounded(2 * M_PI);
        double d = n.kind == Kind::Remote ? 150 : 240;
        n.pos = base + QPointF(d * std::cos(a), d * std::sin(a));
    };

    Node &me = nodes_[selfKey_];
    me.key = selfKey_;
    me.kind = Kind::Self;
    me.id = self.value(QStringLiteral("id")).toString();
    me.name = self.value(QStringLiteral("name")).toString();
    me.arch = self.value(QStringLiteral("arch")).toString();
    me.online = true;
    me.link = QStringLiteral("up");
    me.pinned = true;
    me.pos = QPointF();
    me.present = true;

    // The traffic animation carries on across updates.
    QHash<QString, double> oldPhase;
    for (const Edge &e : edges_)
        oldPhase[e.a < e.b ? e.a + e.b : e.b + e.a] = e.phase;

    QList<Edge> edges;
    QSet<QString> seenPairs;
    auto addEdge = [&](const QString &a, const QString &b, EdgeKind k, double rtt) {
        QString id = a < b ? a + b : b + a;
        if (seenPairs.contains(id))
            return;
        seenPairs.insert(id);
        edges.append({a, b, k, rtt, oldPhase.value(id, QRandomGenerator::global()->generateDouble())});
    };

    QList<QVariantMap> remoteLinks;
    for (const QVariant &v : status_.value(QStringLiteral("peers")).toList()) {
        QVariantMap p = v.toMap();
        bool paired = p.value(QStringLiteral("paired")).toBool();
        bool online = p.value(QStringLiteral("online")).toBool();
        if (!online && !showOffline_)
            continue;
        if (!paired && !showUnpaired_)
            continue;
        QString key = p.value(QStringLiteral("key")).toString();
        bool fresh = !nodes_.contains(key);
        Node &n = nodes_[key];
        n.key = key;
        n.kind = paired ? Kind::Paired : Kind::Seen;
        n.id = p.value(QStringLiteral("id")).toString();
        n.name = p.value(QStringLiteral("name")).toString();
        n.arch = p.value(QStringLiteral("arch")).toString();
        n.online = online;
        n.link = p.value(QStringLiteral("link")).toString();
        n.rtt = p.value(QStringLiteral("rtt")).isNull() ? -1 : p.value(QStringLiteral("rtt")).toDouble();
        n.request = requests.contains(key);
        n.pairing = pairings.contains(key);
        n.renamed = p.value(QStringLiteral("renamed")).toBool();
        n.present = true;
        if (fresh)
            place(n, selfKey_);

        if (n.link == QLatin1String("up"))
            addEdge(selfKey_, key, EdgeKind::Link, n.rtt);
        else if (paired && online)
            addEdge(selfKey_, key, EdgeKind::Connecting, -1);
        else if (online)
            addEdge(selfKey_, key, EdgeKind::Discovery, -1);
        for (const QVariant &nb : p.value(QStringLiteral("neighbors")).toList()) {
            QVariantMap m = nb.toMap();
            m.insert(QStringLiteral("from"), key);
            remoteLinks.append(m);
        }
    }
    // Links between other machines, as their paired machines report them;
    // machines only known that way are drawn small.
    for (const QVariantMap &m : remoteLinks) {
        QString from = m.value(QStringLiteral("from")).toString();
        QString to = m.value(QStringLiteral("key")).toString();
        if (to == selfKey_ || to.isEmpty())
            continue;
        if (!nodes_.contains(to) || (nodes_[to].kind == Kind::Remote && !nodes_[to].present)) {
            bool isNew = !nodes_.contains(to);
            Node &n = nodes_[to];
            n.key = to;
            n.kind = Kind::Remote;
            n.id = m.value(QStringLiteral("id")).toString();
            n.name.clear();
            n.online = true;
            n.link = QStringLiteral("up");
            n.present = true;
            if (isNew)
                place(n, from);
        }
        double rtt = m.value(QStringLiteral("rtt")).toDouble();
        addEdge(from, to, EdgeKind::Remote, rtt);
    }

    for (auto it = nodes_.begin(); it != nodes_.end();) {
        if (!it->present) {
            if (it.key() == selected_)
                setSelected(selfKey_);
            it = nodes_.erase(it);
        } else {
            ++it;
        }
    }
    edges_ = edges;
    if (selected_.isEmpty() || !nodes_.contains(selected_))
        setSelected(selfKey_);
    if (autoFit_)
        fit();
}

void MeshView::step(double dt)
{
    sweep_ = std::fmod(sweep_ + dt * 0.9, 2 * M_PI);
    clock_ += dt;
    for (Edge &e : edges_) {
        // Faster traffic on faster links: 0.25 to 0.9 laps a second.
        double speed = e.rtt < 0 ? 0.35 : qBound(0.25, 0.9 - std::log10(1 + e.rtt) * 0.3, 0.9);
        e.phase = std::fmod(e.phase + dt * speed, 1.0);
    }

    // The layout: nodes repel each other, links pull like springs, a weak
    // pull to the centre keeps the mesh together; this machine stays put.
    const int iterations = 2;
    for (int it = 0; it < iterations; ++it) {
        QHash<QString, QPointF> force;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); ++i)
            for (int j = i + 1; j < keys.size(); ++j) {
                Node &a = nodes_[keys[i]], &b = nodes_[keys[j]];
                QPointF d = a.pos - b.pos;
                double len2 = qMax(QPointF::dotProduct(d, d), 100.0);
                double len = std::sqrt(len2);
                QPointF f = d / len * (26000.0 / len2);
                force[a.key] += f;
                force[b.key] -= f;
            }
        for (const Edge &e : edges_) {
            if (!nodes_.contains(e.a) || !nodes_.contains(e.b))
                continue;
            Node &a = nodes_[e.a], &b = nodes_[e.b];
            QPointF d = b.pos - a.pos;
            double len = qMax(1.0, std::hypot(d.x(), d.y()));
            double rest = e.kind == EdgeKind::Remote ? 170 : e.kind == EdgeKind::Discovery ? 300 : 240;
            QPointF f = d / len * ((len - rest) * 0.035);
            force[a.key] += f;
            force[b.key] -= f;
        }
        for (Node &n : nodes_) {
            if (n.pinned || n.key == dragging_)
                continue;
            QPointF f = force.value(n.key) - n.pos * 0.002;
            n.vel = (n.vel + f) * 0.82;
            double v = std::hypot(n.vel.x(), n.vel.y());
            if (v > 30)
                n.vel *= 30 / v;
            n.pos += n.vel;
        }
    }
}

QPointF MeshView::toScreen(QPointF w) const
{
    return QPointF(width() / 2, height() / 2) + pan_ + w * zoom_;
}

QPointF MeshView::toWorld(QPointF s) const
{
    return (s - QPointF(width() / 2, height() / 2) - pan_) / zoom_;
}

QString MeshView::nodeAt(QPointF screen) const
{
    QString best;
    double bestD = 1e9;
    for (const Node &n : nodes_) {
        double d = std::hypot((toScreen(n.pos) - screen).x(), (toScreen(n.pos) - screen).y());
        if (d <= (radius(n) + 8) * zoom_ && d < bestD) {
            best = n.key;
            bestD = d;
        }
    }
    return best;
}

void MeshView::fit()
{
    if (nodes_.isEmpty() || width() <= 0 || height() <= 0)
        return;
    QRectF box;
    for (const Node &n : nodes_) {
        QRectF r(n.pos - QPointF(radius(n) + 50, radius(n) + 30), QSizeF(2 * radius(n) + 100, 2 * radius(n) + 80));
        box = box.isNull() ? r : box.united(r);
    }
    box = box.united(QRectF(-200, -200, 400, 400));
    double h = qMax(100.0, height() - fitTop_ - fitBottom_);
    double z = qBound(0.35, qMin(width() / box.width(), h / box.height()), 1.3);
    zoom_ = z;
    pan_ = -box.center() * zoom_ + QPointF(0, (fitTop_ - fitBottom_) / 2);
    emit viewChanged();
}

void MeshView::zoomBy(double factor)
{
    autoFit_ = false;
    QPointF c(width() / 2, height() / 2);
    QPointF w = toWorld(c);
    zoom_ = qBound(0.25, zoom_ * factor, 3.0);
    pan_ = c - QPointF(width() / 2, height() / 2) - w * zoom_;
    emit viewChanged();
    update();
}

void MeshView::relayout()
{
    for (Node &n : nodes_) {
        if (n.key == selfKey_)
            continue;
        n.pinned = false;
        double a = QRandomGenerator::global()->bounded(2 * M_PI);
        n.pos = QPointF(240 * std::cos(a), 240 * std::sin(a));
        n.vel = {};
    }
    autoFit_ = true;
    fit();
}

void MeshView::paint(QPainter *p)
{
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::TextAntialiasing);
    drawBackground(p);
    for (const Edge &e : edges_)
        drawEdge(p, e);
    // Smaller ones first, so this machine and the paired ones are on top.
    QList<const Node *> order;
    for (const Node &n : nodes_)
        order.append(&n);
    std::sort(order.begin(), order.end(), [this](const Node *a, const Node *b) { return radius(*a) < radius(*b); });
    for (const Node *n : order)
        drawNode(p, *n);

    // Vignette.
    QRadialGradient vg(QPointF(width() / 2, height() / 2), std::hypot(width(), height()) / 1.6);
    vg.setColorAt(0.6, QColor(0, 0, 0, 0));
    vg.setColorAt(1.0, QColor(0, 0, 0, 150));
    p->fillRect(boundingRect(), vg);
}

void MeshView::drawBackground(QPainter *p)
{
    p->fillRect(boundingRect(), kBg);
    // The grid moves with the view, like a drawing board.
    auto grid = [&](double step, const QColor &c) {
        double s = step * zoom_;
        if (s < 9)
            return;
        p->setPen(QPen(c, 1));
        QPointF o = toScreen(QPointF());
        for (double x = std::fmod(o.x(), s); x < width(); x += s)
            p->drawLine(QPointF(std::floor(x) + 0.5, 0), QPointF(std::floor(x) + 0.5, height()));
        for (double y = std::fmod(o.y(), s); y < height(); y += s)
            p->drawLine(QPointF(0, std::floor(y) + 0.5), QPointF(width(), std::floor(y) + 0.5));
    };
    grid(32, kGridMinor);
    grid(160, kGridMajor);

    if (!nodes_.contains(selfKey_))
        return;
    QPointF c = toScreen(nodes_[selfKey_].pos);
    // Range rings around this machine.
    for (int i = 1; i <= 3; ++i) {
        double r = 160.0 * i * zoom_;
        QPen pen(alpha(kTeal, 34), 1, Qt::DashLine);
        pen.setDashPattern({2, 6});
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(c, r, r);
    }
    // The discovery sweep, while this machine announces itself.
    QString vis = status_.value(QStringLiteral("self")).toMap().value(QStringLiteral("visibility")).toString();
    if (vis == QLatin1String("open")) {
        double r = 480 * zoom_;
        QConicalGradient cg(c, -qRadiansToDegrees(sweep_));
        cg.setColorAt(0.0, alpha(kTeal, 30));
        cg.setColorAt(0.08, alpha(kTeal, 0));
        cg.setColorAt(1.0, alpha(kTeal, 0));
        p->setPen(Qt::NoPen);
        p->setBrush(cg);
        p->drawEllipse(c, r, r);
        QLinearGradient beam(c, c + QPointF(r * std::cos(-sweep_), r * std::sin(-sweep_)));
        beam.setColorAt(0, alpha(kTeal, 110));
        beam.setColorAt(1, alpha(kTeal, 0));
        p->setPen(QPen(QBrush(beam), 1));
        p->drawLine(c, c + QPointF(r * std::cos(-sweep_), r * std::sin(-sweep_)));
    }
}

void MeshView::drawEdge(QPainter *p, const Edge &e)
{
    if (!nodes_.contains(e.a) || !nodes_.contains(e.b))
        return;
    const Node &na = nodes_[e.a], &nb = nodes_[e.b];
    QPointF a = toScreen(na.pos), b = toScreen(nb.pos);
    QPointF d = b - a;
    double len = std::hypot(d.x(), d.y());
    if (len < 1)
        return;
    QPointF u = d / len;
    // From rim to rim.
    a += u * radius(na) * zoom_ * 0.92;
    b -= u * radius(nb) * zoom_ * 0.92;
    bool focus = selected_ == e.a || selected_ == e.b || hover_ == e.a || hover_ == e.b;

    switch (e.kind) {
    case EdgeKind::Link:
    case EdgeKind::Remote: {
        QColor c = e.kind == EdgeKind::Link ? kTeal : alpha(kTeal, 150);
        double w = e.kind == EdgeKind::Link ? 1.8 : 1.2;
        p->setPen(QPen(alpha(c, focus ? 70 : 40), (w + 5) * qMax(0.7, zoom_), Qt::SolidLine, Qt::RoundCap));
        p->drawLine(a, b);
        p->setPen(QPen(c, w * qMax(0.8, zoom_), Qt::SolidLine, Qt::RoundCap));
        p->drawLine(a, b);
        // Traffic: pulses both ways.
        QPointF ab = b - a;
        int pulses = e.kind == EdgeKind::Link ? 3 : 2;
        for (int dir = 0; dir < 2; ++dir)
            for (int k = 0; k < pulses; ++k) {
                double t = std::fmod(e.phase + double(k) / pulses + dir * 0.5 / pulses, 1.0);
                QPointF pos = dir == 0 ? a + ab * t : b - ab * t;
                double fade = std::sin(t * M_PI);
                p->setPen(Qt::NoPen);
                p->setBrush(alpha(QColor(0xc8, 0xff, 0xf8), int(220 * fade)));
                p->drawEllipse(pos, 2.2 * qMax(0.8, zoom_), 2.2 * qMax(0.8, zoom_));
                p->setBrush(alpha(kTeal, int(60 * fade)));
                p->drawEllipse(pos, 5.5 * qMax(0.8, zoom_), 5.5 * qMax(0.8, zoom_));
            }
        // Round-trip time, mid-link.
        if (e.rtt >= 0 && zoom_ > 0.45) {
            QPointF m = (a + b) / 2;
            QFont f = monoFont(10 * qBound(0.85, zoom_, 1.3));
            p->setFont(f);
            QString t = rttText(e.rtt);
            QFontMetricsF fm(f);
            QRectF r(0, 0, fm.horizontalAdvance(t) + 12, fm.height() + 4);
            r.moveCenter(m);
            p->setPen(QPen(alpha(c, focus ? 230 : 140), 1));
            p->setBrush(kPanel);
            p->drawRoundedRect(r, 3, 3);
            p->setPen(focus ? kText : alpha(kText, 200));
            p->drawText(r, Qt::AlignCenter, t);
        }
        break;
    }
    case EdgeKind::Connecting: {
        QPen pen(kAmber, 1.5 * qMax(0.8, zoom_), Qt::CustomDashLine);
        pen.setDashPattern({4, 4});
        pen.setDashOffset(-clock_ * 12);
        p->setPen(pen);
        p->drawLine(a, b);
        break;
    }
    case EdgeKind::Discovery: {
        QPen pen(alpha(kBlue, focus ? 200 : 110), 1.1, Qt::CustomDashLine);
        pen.setDashPattern({1, 5});
        pen.setDashOffset(-clock_ * 6);
        pen.setCapStyle(Qt::RoundCap);
        p->setPen(pen);
        p->drawLine(a, b);
        break;
    }
    }
}

void MeshView::drawNode(QPainter *p, const Node &n)
{
    QPointF c = toScreen(n.pos);
    double r = radius(n) * zoom_;
    if (c.x() < -200 || c.y() < -200 || c.x() > width() + 200 || c.y() > height() + 200)
        return;

    QColor accent;
    switch (n.kind) {
    case Kind::Self: accent = kTeal; break;
    case Kind::Paired:
        accent = !n.online ? kSlate : n.link == QLatin1String("up") ? kTeal : kAmber;
        break;
    case Kind::Seen: accent = n.online ? kBlue : kSlate; break;
    case Kind::Remote: accent = alpha(kTeal, 170); break;
    }
    if (n.request || n.pairing)
        accent = kAmber;
    bool focus = n.key == selected_ || n.key == hover_;

    // Glow.
    if (n.online && n.kind != Kind::Remote) {
        QRadialGradient g(c, r * 2.2);
        g.setColorAt(0, alpha(accent, focus ? 70 : 45));
        g.setColorAt(1, alpha(accent, 0));
        p->setPen(Qt::NoPen);
        p->setBrush(g);
        p->drawEllipse(c, r * 2.2, r * 2.2);
    }
    // A pairing in progress or a request: a pulsing ring.
    if (n.request || n.pairing) {
        double t = std::fmod(clock_, 1.4) / 1.4;
        p->setPen(QPen(alpha(kAmber, int(200 * (1 - t))), 2));
        p->setBrush(Qt::NoBrush);
        p->drawPath(hexagon(c, r * (1.15 + t * 0.6)));
    }

    // The body: a hexagon, dark, with the state's colour on its rim.
    QLinearGradient body(c - QPointF(0, r), c + QPointF(0, r));
    body.setColorAt(0, QColor(0x16, 0x21, 0x2c));
    body.setColorAt(1, QColor(0x0a, 0x10, 0x16));
    QPen rim(accent, (n.kind == Kind::Self ? 2.4 : 1.8) * qMax(0.8, zoom_));
    if (!n.online)
        rim.setStyle(Qt::DashLine);
    p->setPen(rim);
    p->setBrush(body);
    p->drawPath(hexagon(c, r));
    if (n.kind == Kind::Self) {
        p->setPen(QPen(alpha(accent, 120), 1));
        p->setBrush(Qt::NoBrush);
        p->drawPath(hexagon(c, r * 1.22));
    }
    // Inner core.
    p->setPen(Qt::NoPen);
    p->setBrush(alpha(accent, n.online ? 46 : 22));
    p->drawPath(hexagon(c, r * 0.58));
    p->setBrush(n.online ? accent : kSlate);
    p->drawEllipse(c, r * 0.13, r * 0.13);

    // Status light, top right.
    if (n.kind != Kind::Remote) {
        QColor led = n.kind == Kind::Self ? kTeal
                     : !n.online           ? kSlate
                     : n.kind == Kind::Seen ? kBlue
                     : n.link == QLatin1String("up") ? QColor(0x3d, 0xe0, 0x7a)
                                                     : kAmber;
        QPointF lp = c + QPointF(r * 0.78, -r * 0.62);
        p->setBrush(kBg);
        p->drawEllipse(lp, 4.6 * qMax(0.8, zoom_), 4.6 * qMax(0.8, zoom_));
        p->setBrush(led);
        p->drawEllipse(lp, 3.0 * qMax(0.8, zoom_), 3.0 * qMax(0.8, zoom_));
    }

    // Selection: targeting brackets.
    if (n.key == selected_) {
        double s = r + 9 * qMax(0.8, zoom_), l = 7 * qMax(0.8, zoom_);
        p->setPen(QPen(kText, 1.4));
        for (int sx : {-1, 1})
            for (int sy : {-1, 1}) {
                QPointF k = c + QPointF(sx * s, sy * s);
                p->drawLine(k, k - QPointF(sx * l, 0));
                p->drawLine(k, k - QPointF(0, sy * l));
            }
    }

    // Labels.
    if (zoom_ < 0.4 && n.kind == Kind::Remote)
        return;
    double y = c.y() + r + 8 * qMax(0.8, zoom_);
    QString name = n.name.isEmpty() ? n.id : n.name;
    if (name.size() > 22)
        name = name.left(21) + QStringLiteral("…");
    QFont nf = uiFont((n.kind == Kind::Remote ? 10 : 12) * qBound(0.8, zoom_, 1.3), QFont::DemiBold);
    p->setFont(nf);
    QFontMetricsF fm(nf);
    // Labels on a dark plate, so links passing under stay readable.
    auto plate = [&](double w, double h, double top) {
        p->setPen(Qt::NoPen);
        p->setBrush(alpha(kBg, 190));
        p->drawRoundedRect(QRectF(c.x() - w / 2 - 4, top, w + 8, h), 2, 2);
    };
    plate(fm.horizontalAdvance(name), fm.height(), y);
    p->setPen(n.online ? kText : kDim);
    p->drawText(QPointF(c.x() - fm.horizontalAdvance(name) / 2, y + fm.ascent()), name);
    y += fm.height();
    if (!n.name.isEmpty()) {
        QFont mf = monoFont(9.5 * qBound(0.8, zoom_, 1.3));
        p->setFont(mf);
        QFontMetricsF mm(mf);
        QString sub = n.id;
        if (n.kind == Kind::Self)
            sub = QStringLiteral("THIS NODE · ") + n.id;
        else if (n.request)
            sub = QStringLiteral("REQUEST · ") + n.id;
        else if (n.renamed)
            sub = QStringLiteral("RENAMED · ") + n.id;
        plate(mm.horizontalAdvance(sub), mm.height(), y);
        p->setPen(n.request || n.renamed ? kAmber : kDim);
        p->drawText(QPointF(c.x() - mm.horizontalAdvance(sub) / 2, y + mm.ascent()), sub);
    }
}

void MeshView::mousePressEvent(QMouseEvent *e)
{
    pressAt_ = lastMouse_ = e->position();
    moved_ = false;
    dragging_ = nodeAt(e->position());
    if (dragging_ == selfKey_)
        dragging_.clear();
    panning_ = dragging_.isEmpty();
    e->accept();
}

void MeshView::mouseMoveEvent(QMouseEvent *e)
{
    QPointF d = e->position() - lastMouse_;
    lastMouse_ = e->position();
    if (!moved_ && std::hypot((e->position() - pressAt_).x(), (e->position() - pressAt_).y()) < 4)
        return;
    moved_ = true;
    if (!dragging_.isEmpty() && nodes_.contains(dragging_)) {
        Node &n = nodes_[dragging_];
        n.pos = toWorld(e->position());
        n.vel = {};
        n.pinned = true;
    } else if (panning_) {
        pan_ += d;
        autoFit_ = false;
        emit viewChanged();
    }
    update();
}

void MeshView::mouseReleaseEvent(QMouseEvent *e)
{
    if (!moved_) {
        QString k = nodeAt(e->position());
        setSelected(k.isEmpty() ? selfKey_ : k);
    }
    dragging_.clear();
    panning_ = false;
}

void MeshView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QString k = nodeAt(e->position());
    if (!k.isEmpty() && k != selfKey_ && nodes_.contains(k)) {
        nodes_[k].pinned = false;   // back into the layout
        return;
    }
    autoFit_ = true;
    fit();
}

void MeshView::wheelEvent(QWheelEvent *e)
{
    autoFit_ = false;
    QPointF at = e->position();
    QPointF w = toWorld(at);
    double f = std::pow(1.0015, e->angleDelta().y());
    zoom_ = qBound(0.25, zoom_ * f, 3.0);
    pan_ = at - QPointF(width() / 2, height() / 2) - w * zoom_;
    emit viewChanged();
    update();
}

void MeshView::hoverMoveEvent(QHoverEvent *e)
{
    QString k = nodeAt(e->position());
    if (k != hover_) {
        hover_ = k;
        setCursor(k.isEmpty() ? Qt::ArrowCursor : Qt::PointingHandCursor);
        update();
    }
}

void MeshView::hoverLeaveEvent(QHoverEvent *)
{
    hover_.clear();
    update();
}
