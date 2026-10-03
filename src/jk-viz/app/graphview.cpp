#include "graphview.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

namespace {
// Layer colours, top to bottom.
const QColor kLayer[6] = {QColor("#d8a83a"), QColor("#b48ae0"), QColor("#e5737d"),
                          QColor("#62a8ec"), QColor("#4fc1c9"), QColor("#8fcf6e")};
const QColor kBg("#0e1116"), kNode("#191e26"), kHot("#ff5f3d"), kText("#e6e9ee"), kDim("#8b94a3");

QColor edgeColor(const QString &t)
{
    if (t == "socket")
        return QColor("#5aa7f0");
    if (t == "dbus")
        return QColor("#c67be0");
    if (t == "pipe")
        return QColor("#8fd16a");
    if (t == "tcp")
        return QColor("#e8c25c");
    if (t == "net")
        return QColor("#f0954a");
    if (t == "dev")
        return QColor("#a9b4c4");
    if (t == "hw")
        return QColor("#e5737d");
    return QColor("#4a5260");
}

QColor mix(const QColor &a, const QColor &b, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return QColor::fromRgbF(float(a.redF() + (b.redF() - a.redF()) * t), float(a.greenF() + (b.greenF() - a.greenF()) * t),
                            float(a.blueF() + (b.blueF() - a.blueF()) * t), float(a.alphaF() + (b.alphaF() - a.alphaF()) * t));
}

QColor alpha(QColor c, int a)
{
    c.setAlpha(a);
    return c;
}
} // namespace

GraphView::GraphView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
    setFillColor(kBg);
}

void GraphView::setModel(GraphModel *m)
{
    if (m == model_)
        return;
    if (model_)
        disconnect(model_, nullptr, this, nullptr);
    model_ = m;
    if (model_) {
        connect(model_, &GraphModel::updated, this, [this] {
            if (!placed_ && !model_->nodes().empty()) {
                placed_ = true;
                home();
            }
            update();
        });
        connect(model_, &GraphModel::layoutChanged, this, [this] { update(); });
    }
    emit modelChanged();
    update();
}

// ---------------------------------------------------------------- view
void GraphView::fit()
{
    if (!model_ || model_->bounds().isNull())
        return;
    QRectF b = model_->bounds().adjusted(-40, -40, 40, 40);
    zoom_ = std::clamp(std::min(width() / b.width(), height() / b.height()), 0.03, 2.0);
    pan_ = QPointF(width() / 2, height() / 2) - b.center() * zoom_;
    emit viewChanged();
    update();
}

void GraphView::home()
{
    // The top of the stack (silicon), centred on the kernel, readable size.
    if (!model_)
        return;
    zoom_ = 0.62;
    int k = model_->indexOf("kernel");
    double cx = k >= 0 ? model_->nodeRect(k).center().x() : model_->bounds().center().x();
    pan_ = QPointF(width() / 2 - cx * zoom_, 30 - model_->bounds().top() * zoom_);
    emit viewChanged();
    update();
}

void GraphView::centerOnNode(const QString &id)
{
    if (!model_)
        return;
    int i = model_->visibleOf(model_->indexOf(id));
    if (i < 0)
        return;
    if (zoom_ < 0.45)
        zoom_ = 0.7;
    pan_ = QPointF(width() / 2, height() / 2) - model_->nodeRect(i).center() * zoom_;
    emit viewChanged();
    update();
}

void GraphView::zoomBy(double f)
{
    QPointF c(width() / 2, height() / 2);
    QPointF s = toScene(c);
    zoom_ = std::clamp(zoom_ * f, 0.03, 3.0);
    pan_ = c - s * zoom_;
    emit viewChanged();
    update();
}

int GraphView::hit(QPointF pos) const
{
    if (!model_)
        return -1;
    QPointF s = toScene(pos);
    const auto &nodes = model_->nodes();
    for (size_t i = 0; i < nodes.size(); i++)
        if (nodes[i].visible && model_->nodeRect(int(i)).contains(s))
            return int(i);
    return -1;
}

void GraphView::mousePressEvent(QMouseEvent *e)
{
    press_ = e->position();
    panAtPress_ = pan_;
    dragging_ = false;
    e->accept();
}

void GraphView::mouseMoveEvent(QMouseEvent *e)
{
    QPointF d = e->position() - press_;
    if (!dragging_ && std::hypot(d.x(), d.y()) > 4)
        dragging_ = true;
    if (dragging_) {
        pan_ = panAtPress_ + d;
        update();
    }
}

void GraphView::mouseReleaseEvent(QMouseEvent *e)
{
    if (dragging_ || !model_)
        return;
    int i = hit(e->position());
    if (e->button() == Qt::RightButton) {
        if (i >= 0)
            model_->toggleCollapse(model_->nodes()[size_t(i)].id);
        return;
    }
    model_->select(i >= 0 ? model_->nodes()[size_t(i)].id : QString());
}

void GraphView::mouseDoubleClickEvent(QMouseEvent *e)
{
    int i = hit(e->position());
    if (i >= 0 && model_)
        model_->toggleCollapse(model_->nodes()[size_t(i)].id);
}

void GraphView::wheelEvent(QWheelEvent *e)
{
    double steps = e->angleDelta().y() / 120.0;
    if (e->modifiers() & Qt::ShiftModifier || std::abs(e->angleDelta().x()) > std::abs(e->angleDelta().y())) {
        // Sideways: pan.
        pan_ += QPointF(e->angleDelta().x() ? e->angleDelta().x() : e->angleDelta().y(), 0) * 0.8;
        update();
        return;
    }
    QPointF s = toScene(e->position());
    zoom_ = std::clamp(zoom_ * std::pow(1.15, steps), 0.03, 3.0);
    pan_ = e->position() - s * zoom_;
    emit viewChanged();
    update();
}

void GraphView::hoverMoveEvent(QHoverEvent *e)
{
    int h = hit(e->position());
    if (h != hover_) {
        hover_ = h;
        update();
    }
}

// ---------------------------------------------------------------- drawing
void GraphView::paint(QPainter *p)
{
    if (!model_ || model_->nodes().empty())
        return;
    const auto &nodes = model_->nodes();
    p->setRenderHint(QPainter::Antialiasing);
    QRectF view(toScene(QPointF(0, 0)), toScene(QPointF(width(), height())));
    QRectF bounds = model_->bounds();
    const int sel = model_->selectedIndex();

    p->save();
    p->translate(pan_);
    p->scale(zoom_, zoom_);

    // Layer bands.
    auto bands = model_->bands();
    for (int L = 0; L < 6; L++) {
        auto [top, bottom] = bands[size_t(L)];
        if (top == 0 && bottom == 0)
            continue;
        QRectF r(std::min(view.left(), bounds.left() - 200), top, std::max(view.width(), bounds.width() + 400) + 400,
                 bottom - top);
        p->fillRect(r, alpha(kLayer[L], L % 2 ? 14 : 22));
        p->setPen(QPen(alpha(kLayer[L], 60), 1 / zoom_));
        p->drawLine(QPointF(r.left(), top), QPointF(r.right(), top));
    }

    // Who is connected to the selection (to light them up).
    std::vector<char> lit(nodes.size(), 0);
    if (sel >= 0)
        for (const auto &e : model_->edges())
            if (model_->edgeShown(e.type) && (e.a == sel || e.b == sel)) {
                int a = model_->visibleOf(e.a), b = model_->visibleOf(e.b);
                if (a >= 0)
                    lit[size_t(a)] = 1;
                if (b >= 0)
                    lit[size_t(b)] = 1;
            }

    // Parent -> child.
    if (model_->edgeShown("tree")) {
        QPen pen(QColor("#3b4350"), 1.3);
        pen.setCosmetic(zoom_ < 0.5);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        for (size_t i = 0; i < nodes.size(); i++) {
            const VNode &n = nodes[i];
            if (!n.visible || n.parent < 0)
                continue;
            QRectF a = model_->nodeRect(n.parent), b = model_->nodeRect(int(i));
            if (!view.intersects(a.united(b)))
                continue;
            QPointF s(a.center().x(), a.bottom()), t(b.center().x(), b.top());
            QPainterPath path(s);
            double dy = (t.y() - s.y()) * 0.5;
            path.cubicTo(s + QPointF(0, dy), t - QPointF(0, dy), t);
            p->drawPath(path);
        }
    }

    // Sockets, D-Bus, pipes, TCP, network, devices, drivers.
    QFont small = p->font();
    small.setPixelSize(10);
    struct Label { QPointF at; QString text; QColor color; };
    std::vector<Label> labels;
    for (const auto &e : model_->edges()) {
        if (!model_->edgeShown(e.type))
            continue;
        int a = model_->visibleOf(e.a), b = model_->visibleOf(e.b);
        if (a < 0 || b < 0 || a == b)
            continue;
        QRectF ra = model_->nodeRect(a), rb = model_->nodeRect(b);
        if (!view.intersects(ra.united(rb)))
            continue;
        bool on = sel >= 0 && (e.a == sel || e.b == sel || a == sel || b == sel);
        bool dim = sel >= 0 && !on;
        QColor c = edgeColor(e.type);
        QPen pen(alpha(c, on ? 235 : dim ? 14 : 55), (on ? 2.4 : 1.2) * (e.count > 3 ? 1.4 : 1.0));
        if (e.type == "dev" || e.type == "hw")
            pen.setStyle(Qt::DashLine);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        // From the facing sides; sideways between nodes on one row.
        QPointF s, t, c1, c2;
        if (std::abs(ra.center().y() - rb.center().y()) < 1) {
            bool right = rb.center().x() > ra.center().x();
            s = QPointF(right ? ra.right() : ra.left(), ra.center().y());
            t = QPointF(right ? rb.left() : rb.right(), rb.center().y());
            double bow = std::min(160.0, std::abs(t.x() - s.x()) * 0.25) + 18;
            c1 = s + QPointF(0, -bow);
            c2 = t + QPointF(0, -bow);
            s.ry() = ra.top() + 8;
            t.ry() = rb.top() + 8;
        } else {
            bool down = rb.center().y() > ra.center().y();
            s = QPointF(ra.center().x() + 30, down ? ra.bottom() : ra.top());
            t = QPointF(rb.center().x() - 30, down ? rb.top() : rb.bottom());
            double dy = (t.y() - s.y()) * 0.45;
            double side = (t.x() - s.x()) * 0.1;
            c1 = s + QPointF(side, dy);
            c2 = t - QPointF(side, dy);
        }
        QPainterPath path(s);
        path.cubicTo(c1, c2, t);
        p->drawPath(path);
        bool directed = e.type == "pipe" || e.type == "dev" || e.type == "hw";
        if (directed && !dim) {
            QPointF dir = t - path.pointAtPercent(0.95);
            double len = std::hypot(dir.x(), dir.y());
            if (len > 0.01) {
                dir /= len;
                QPointF n(-dir.y(), dir.x());
                QPolygonF head({t, t - dir * 9 + n * 4.5, t - dir * 9 - n * 4.5});
                p->setPen(Qt::NoPen);
                p->setBrush(pen.color());
                p->drawPolygon(head);
            }
        }
        if (on && zoom_ > 0.3) {
            QString l = e.type == "dbus" ? "D-Bus" : e.type;
            if (!e.label.isEmpty())
                l += ": " + e.label;
            if (e.count > 1)
                l += QString(" ×%1").arg(e.count);
            labels.push_back({path.pointAtPercent(0.5), l, c});
        }
    }

    // Nodes.
    QFont bold = p->font();
    bold.setPixelSize(12);
    bold.setBold(true);
    QFont normal = p->font();
    normal.setPixelSize(11);
    QFontMetricsF fmBold(bold), fmNormal(normal), fmSmall(small);
    const double maxShare = std::max(model_->maxShare(), 0.02);
    const bool text = zoom_ > 0.28;
    for (size_t i = 0; i < nodes.size(); i++) {
        const VNode &n = nodes[i];
        if (!n.visible)
            continue;
        QRectF r = model_->nodeRect(int(i));
        if (!view.intersects(r))
            continue;
        QColor lc = kLayer[std::clamp(n.layer, 0, 5)];
        double share = model_->share(n, n.collapsed);
        double heat = n.hw ? 0 : std::pow(std::min(1.0, share / maxShare), 0.6);
        bool faded = sel >= 0 && int(i) != sel && !lit[i];
        QColor fill = mix(mix(kNode, lc, 0.10), kHot, heat * 0.55);
        if (n.hw) {
            // Devices: their own load, as a softer tint.
            double load = 0;
            int res = model_->resource();
            if (res == Cpu || res == Gpu)
                load = n.res[res];
            fill = mix(mix(kNode, lc, 0.14), lc, load * 0.45);
        }
        if (faded)
            fill = mix(fill, kBg, 0.55);
        QPen border(alpha(lc, faded ? 70 : 190), 1.4);
        if (int(i) == sel)
            border = QPen(Qt::white, 3);
        else if (n.match)
            border = QPen(QColor("#ffd84a"), 3);
        else if (int(i) == hover_)
            border = QPen(lc.lighter(140), 2);
        else if (lit[i])
            border = QPen(lc.lighter(130), 2.2);
        p->setPen(border);
        p->setBrush(fill);
        p->drawRoundedRect(r, 7, 7);
        // Share of 100%: a bar along the bottom.
        if (!n.hw && share > 0) {
            QRectF bar(r.left() + 6, r.bottom() - 6, std::max(2.0, (r.width() - 12) * std::min(1.0, share)), 3);
            p->setPen(Qt::NoPen);
            p->setBrush(mix(lc, kHot, heat));
            p->drawRoundedRect(bar, 1.5, 1.5);
        }
        if (!text)
            continue;
        QColor tc = faded ? kDim : kText;
        p->setFont(bold);
        p->setPen(tc);
        QString title = n.name;
        QString tag = n.pid >= 0 ? QString::number(n.pid) : n.hw ? n.kind.toUpper() : QString();
        double tagW = tag.isEmpty() ? 0 : fmSmall.horizontalAdvance(tag) + 8;
        p->drawText(QRectF(r.left() + 8, r.top() + 4, r.width() - 16 - tagW, 18), Qt::AlignLeft | Qt::AlignVCenter,
                    fmBold.elidedText(title, Qt::ElideRight, r.width() - 16 - tagW));
        if (!tag.isEmpty()) {
            p->setFont(small);
            p->setPen(alpha(lc, faded ? 110 : 220));
            p->drawText(QRectF(r.right() - tagW - 6, r.top() + 4, tagW, 18), Qt::AlignRight | Qt::AlignVCenter, tag);
        }
        // The value line: share and amount, or the device's load.
        QString line;
        if (n.hw) {
            line = model_->deviceLoad(n);
            if (line.isEmpty())
                line = n.info.section(" · ", 0, 0);
        } else {
            double own = model_->share(n);
            line = QString::number(own * 100, own < 0.1 ? 'f' : 'f', own < 0.1 ? 2 : 1) + "%";
            double v = n.res[model_->resource()];
            if (v > 0 && model_->resource() != Cpu && model_->resource() != Gpu)
                line += "  " + model_->format(model_->resource(), v);
            if (!n.children.empty()) {
                double sub = model_->share(n, true);
                if (std::abs(sub - own) > 0.0005)
                    line += QString("  Σ%1%").arg(sub * 100, 0, 'f', 1);
            }
        }
        p->setFont(normal);
        p->setPen(faded ? kDim : QColor("#b9c1cc"));
        double badgeW = 0;
        if (n.collapsed && !n.children.empty()) {
            QString badge = QString("+%1").arg(n.children.size());
            badgeW = fmSmall.horizontalAdvance(badge) + 10;
            QRectF br(r.right() - badgeW - 5, r.top() + 24, badgeW, 15);
            p->setPen(Qt::NoPen);
            p->setBrush(alpha(lc, 150));
            p->drawRoundedRect(br, 7, 7);
            p->setFont(small);
            p->setPen(kBg);
            p->drawText(br, Qt::AlignCenter, badge);
            p->setFont(normal);
            p->setPen(faded ? kDim : QColor("#b9c1cc"));
        }
        p->drawText(QRectF(r.left() + 8, r.top() + 22, r.width() - 16 - badgeW, 17), Qt::AlignLeft | Qt::AlignVCenter,
                    fmNormal.elidedText(line, Qt::ElideRight, r.width() - 16 - badgeW));
    }

    // Labels of the selection's connections, on top.
    p->setFont(small);
    for (auto &l : labels) {
        QString t = fmSmall.elidedText(l.text, Qt::ElideRight, 260);
        QRectF br(0, 0, fmSmall.horizontalAdvance(t) + 10, 16);
        br.moveCenter(l.at);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(14, 17, 22, 220));
        p->drawRoundedRect(br, 4, 4);
        p->setPen(l.color);
        p->drawText(br, Qt::AlignCenter, t);
    }
    p->restore();

    // Layer names, pinned to the left edge.
    QFont lf = p->font();
    lf.setPixelSize(12);
    lf.setBold(true);
    lf.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    p->setFont(lf);
    for (int L = 0; L < 6; L++) {
        auto [top, bottom] = bands[size_t(L)];
        if (top == 0 && bottom == 0)
            continue;
        double y0 = pan_.y() + top * zoom_, y1 = pan_.y() + bottom * zoom_;
        if (y1 < 0 || y0 > height())
            continue;
        double y = std::min(std::max(8.0, y0 + 6), y1 - 22);
        QString name = QString("%1  %2").arg(L).arg(GraphModel::layerName(L).toUpper());
        QRectF br(8, y, QFontMetricsF(lf).horizontalAdvance(name) + 16, 20);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(14, 17, 22, 200));
        p->drawRoundedRect(br, 5, 5);
        p->setPen(kLayer[L]);
        p->drawText(br, Qt::AlignCenter, name);
    }

    // Hover: the full name and command.
    if (hover_ >= 0 && size_t(hover_) < nodes.size() && nodes[size_t(hover_)].visible) {
        const VNode &n = nodes[size_t(hover_)];
        QString t = n.name + (n.pid >= 0 ? QString("  (pid %1, %2)").arg(n.pid).arg(n.user) : QString());
        QString t2 = !n.cmd.isEmpty() ? n.cmd : n.info;
        QFont f = p->font();
        f.setPixelSize(12);
        f.setBold(false);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        p->setFont(f);
        QFontMetricsF fm(f);
        t2 = fm.elidedText(t2, Qt::ElideRight, 560);
        double w = std::max(fm.horizontalAdvance(t), fm.horizontalAdvance(t2)) + 16;
        QRectF r = model_->nodeRect(hover_);
        QPointF at = pan_ + QPointF(r.left(), r.bottom()) * zoom_ + QPointF(0, 6);
        QRectF box(at, QSizeF(w, t2.isEmpty() ? 24 : 42));
        if (box.right() > width())
            box.moveRight(width() - 4);
        if (box.bottom() > height())
            box.moveBottom(pan_.y() + r.top() * zoom_ - 6);
        p->setPen(QColor("#3b4350"));
        p->setBrush(QColor(24, 28, 36, 245));
        p->drawRoundedRect(box, 6, 6);
        p->setPen(kText);
        p->drawText(box.adjusted(8, 4, -8, 0), Qt::AlignLeft | Qt::AlignTop, t);
        p->setPen(kDim);
        p->drawText(box.adjusted(8, 22, -8, 0), Qt::AlignLeft | Qt::AlignTop, t2);
    }
}
