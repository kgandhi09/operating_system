#include "graphmodel.h"
#include "client.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>

namespace {
const char *kResKeys[ResourceCount] = {"cpu", "mem", "gpu", "vram", "io", "net", "pwr"};
const char *kResNames[ResourceCount] = {"CPU", "Memory", "GPU", "VRAM", "Disk I/O", "Network", "Power"};
constexpr int GridMin = 6;      // more leaves than this under one node: a grid

int kindOrder(const QString &k)
{
    static const QStringList order = {"cpu", "core", "ram", "gpu", "storage", "disk", "nic", "wireless", "netif",
                                      "usbhost", "usbhub", "usb", "media", "bus", "pci", "firmware", "kernel",
                                      "idle", "other", "kmem", "driver", "kthread", "kthreads", "process"};
    int i = int(order.indexOf(k));
    return i < 0 ? 99 : i;
}

QString bytes(double v)
{
    const char *u[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    int i = 0;
    while (v >= 1024 && i < 4) {
        v /= 1024;
        i++;
    }
    return QString::number(v, 'f', i == 0 ? 0 : v < 10 ? 2 : v < 100 ? 1 : 0) + " " + u[i];
}

QString rate(double v)
{
    const char *u[] = {"B/s", "kB/s", "MB/s", "GB/s"};
    int i = 0;
    while (v >= 1000 && i < 3) {
        v /= 1000;
        i++;
    }
    return QString::number(v, 'f', i == 0 ? 0 : v < 10 ? 2 : v < 100 ? 1 : 0) + " " + u[i];
}
} // namespace

RowsModel::RowsModel(const QList<QByteArray> &roles, QObject *parent)
    : QAbstractListModel(parent)
    , roles_(roles)
{
}

QVariant RowsModel::data(const QModelIndex &index, int role) const
{
    int r = role - Qt::UserRole;
    if (!index.isValid() || index.row() >= rows_.size() || r < 0 || r >= roles_.size())
        return {};
    return rows_[index.row()].value(QString::fromLatin1(roles_[r]));
}

QHash<int, QByteArray> RowsModel::roleNames() const
{
    QHash<int, QByteArray> h;
    for (qsizetype i = 0; i < roles_.size(); i++)
        h.insert(Qt::UserRole + int(i), roles_[i]);
    return h;
}

void RowsModel::setRows(const QList<QVariantMap> &rows)
{
    if (rows.size() == rows_.size()) {
        rows_ = rows;
        if (!rows_.isEmpty())
            emit dataChanged(index(0), index(int(rows_.size()) - 1));
        return;
    }
    beginResetModel();
    rows_ = rows;
    endResetModel();
    emit countChanged();
}

GraphModel::GraphModel(QObject *parent)
    : QObject(parent)
    , sel_(new QQmlPropertyMap(this))
    , selResources_(new RowsModel({"name", "value", "share", "sub", "current"}, this))
    , selLinks_(new RowsModel({"type", "nodeId", "name", "label", "dir", "count"}, this))
    , selMembers_(new RowsModel({"line"}, this))
    , top_(new RowsModel({"nodeId", "name", "pid", "layerIndex", "share", "value"}, this))
    , totals_m_(new RowsModel({"resource", "name", "summary", "detail"}, this))
{
    sel_->insert(QStringLiteral("id"), QString());
    QString file = qEnvironmentVariable("JK_VIZ_FILE");
    client_ = new VizClient(this);
    connect(client_, &VizClient::statusChanged, this, &GraphModel::statusChanged);
    connect(client_, &VizClient::snapshot, this, &GraphModel::ingest);
    if (!file.isEmpty())
        loadFile(file);
}

GraphModel::~GraphModel()
{
    // The client first, while this is still whole (it signals as it goes).
    disconnect(client_, nullptr, this, nullptr);
    delete client_;
}

QString GraphModel::status() const
{
    if (paused_)
        return QStringLiteral("paused");
    return client_->status();
}

QStringList GraphModel::resourceNames() const
{
    QStringList l;
    for (auto *n : kResNames)
        l << QString::fromLatin1(n);
    return l;
}

QString GraphModel::layerName(int l)
{
    static const char *names[] = {"Silicon", "Firmware", "Kernel", "System services", "Sessions", "Applications"};
    return l >= 0 && l < 6 ? QString::fromLatin1(names[l]) : QString();
}

void GraphModel::setResource(int r)
{
    if (r == resource_ || r < 0 || r >= ResourceCount)
        return;
    resource_ = r;
    computeTop();
    emit resourceChanged();
    emit updated();
    computeSelected();
    emit selectionChanged();
}

void GraphModel::setOfUsed(bool b)
{
    if (b == ofUsed_)
        return;
    ofUsed_ = b;
    computeTop();
    emit resourceChanged();
    emit updated();
    computeSelected();
    emit selectionChanged();
}

void GraphModel::setPaused(bool b)
{
    if (b == paused_)
        return;
    paused_ = b;
    emit pausedChanged();
    emit statusChanged();
    if (!b && !last_.isEmpty())
        rebuild();
}

void GraphModel::setSearch(const QString &s)
{
    if (s == search_)
        return;
    search_ = s;
    matchCursor_ = 0;
    applySearch();
    emit searchChanged();
    emit updated();
}

void GraphModel::loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return;
    QJsonDocument d = QJsonDocument::fromJson(f.readLine());
    if (d.isObject())
        ingest(d.object());
}

void GraphModel::ingest(const QJsonObject &s)
{
    last_ = s;
    if (!paused_)
        rebuild();
}

// ---------------------------------------------------------------- snapshot -> graph
void GraphModel::rebuild()
{
    const QJsonObject &s = last_;
    host_ = s.value("host").toString();
    interval_ = s.value("interval").toDouble();
    std::vector<VNode> nodes;
    QHash<QString, int> index;
    const QJsonArray jn = s.value("nodes").toArray();
    nodes.reserve(size_t(jn.size()));
    for (const auto &v : jn) {
        QJsonObject o = v.toObject();
        VNode n;
        n.id = o.value("id").toString();
        n.kind = o.value("kind").toString();
        n.name = o.value("name").toString();
        n.parentId = o.value("parent").toString();
        n.user = o.value("user").toString();
        n.cmd = o.value("cmd").toString();
        n.state = o.value("state").toString();
        n.info = o.value("info").toString();
        n.layer = o.value("layer").toInt();
        n.pid = o.value("pid").toInt(-1);
        n.threads = o.value("threads").toInt();
        n.fds = o.value("fds").toInt();
        n.hw = o.value("hw").toBool();
        n.irqs = o.value("irqs").toDouble();
        QJsonObject r = o.value("r").toObject();
        for (int k = 0; k < ResourceCount; k++)
            n.res[k] = r.value(kResKeys[k]).toDouble();
        for (const auto &m : o.value("members").toArray())
            n.members << m.toString();
        n.collapsed = collapsed_.contains(n.id);
        index.insert(n.id, int(nodes.size()));
        nodes.push_back(std::move(n));
    }
    // The tree.
    int machine = index.value("machine", -1), kernel = index.value("kernel", -1);
    for (size_t i = 0; i < nodes.size(); i++) {
        VNode &n = nodes[i];
        int p = index.value(n.parentId, -1);
        if (p < 0 && int(i) != machine)
            p = n.layer >= 2 && kernel >= 0 && int(i) != kernel ? kernel : machine;
        if (p == int(i))
            p = -1;
        n.parent = p;
        if (p >= 0)
            nodes[size_t(p)].children.push_back(int(i));
    }
    for (auto &n : nodes)
        std::sort(n.children.begin(), n.children.end(), [&](int a, int b) {
            const VNode &x = nodes[size_t(a)], &y = nodes[size_t(b)];
            if (x.layer != y.layer)
                return x.layer < y.layer;
            int kx = kindOrder(x.kind), ky = kindOrder(y.kind);
            if (kx != ky)
                return kx < ky;
            int c = QString::compare(x.name, y.name, Qt::CaseInsensitive);
            if (c)
                return c < 0;
            return x.pid != y.pid ? x.pid < y.pid : x.id < y.id;
        });
    // Sums over each subtree (not over the hardware: a device's load is not a share).
    std::vector<int> order;
    std::deque<int> q;
    for (size_t i = 0; i < nodes.size(); i++)
        if (nodes[i].parent < 0)
            q.push_back(int(i));
    while (!q.empty()) {
        int i = q.front();
        q.pop_front();
        order.push_back(i);
        for (int c : nodes[size_t(i)].children) {
            nodes[size_t(c)].depth = nodes[size_t(i)].depth + 1;
            q.push_back(c);
        }
    }
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        VNode &n = nodes[size_t(*it)];
        for (int k = 0; k < ResourceCount; k++)
            n.sub[k] = n.hw ? 0 : n.res[k];
        for (int c : n.children)
            for (int k = 0; k < ResourceCount; k++)
                n.sub[k] += nodes[size_t(c)].sub[k];
        if (n.hw)
            for (int k = 0; k < ResourceCount; k++)
                n.sub[k] = n.res[k];
    }
    // Totals: 100% of each resource.
    QJsonObject t = s.value("totals").toObject();
    cpus_ = t.value("cpus").toInt();
    gpus_ = t.value("gpus").toInt();
    cpuW_ = t.value("cpuW").toDouble();
    gpuW_ = t.value("gpuW").toDouble();
    havePower_ = t.value("power").toBool();
    netCaptured_ = t.value("netCaptured").toBool();
    netDrops_ = t.value("netDrops").toDouble();
    totals_[Cpu] = 1;
    totals_[Mem] = t.value("mem").toDouble();
    totals_[Gpu] = gpus_ > 0 ? 1 : 0;
    totals_[Vram] = t.value("vram").toDouble();
    totals_[Io] = t.value("io").toDouble();
    totals_[Net] = t.value("net").toDouble();
    totals_[Pwr] = cpuW_ + gpuW_;
    int idle = index.value("idle", -1);
    for (int k = 0; k < ResourceCount; k++)
        used_[k] = std::max(0.0, totals_[k] - (idle >= 0 ? nodes[size_t(idle)].res[k] : 0));

    std::vector<VEdge> edges;
    for (const auto &v : s.value("edges").toArray()) {
        QJsonObject o = v.toObject();
        VEdge e;
        e.a = index.value(o.value("a").toString(), -1);
        e.b = index.value(o.value("b").toString(), -1);
        if (e.a < 0 || e.b < 0)
            continue;
        e.type = o.value("type").toString();
        e.label = o.value("label").toString();
        e.count = o.value("n").toInt(1);
        edges.push_back(std::move(e));
    }
    nodes_ = std::move(nodes);
    edges_ = std::move(edges);
    index_ = std::move(index);
    selected_ = index_.value(selectedKey_, -1);
    applySearch();
    layout();
    computeTop();
    computeTotals();
    emit updated();
    computeSelected();
    emit selectionChanged();
}

// ---------------------------------------------------------------- layout
// Layers top to bottom, each a band of rows, all centred on one axis.
// Inside a layer, a tidy tree: every subtree gets its own columns, a node
// sits over its children, and many leaves under one node are packed into a
// grid. A layer's trees start under their parents in the layer above (the
// parent -> child line crosses into the band), in the order of those
// parents, left to right.
void GraphModel::layout()
{
    const size_t N = nodes_.size();
    std::vector<int> order;
    std::deque<int> q;
    for (size_t i = 0; i < N; i++) {
        nodes_[i].visible = false;
        if (nodes_[i].parent < 0)
            q.push_back(int(i));
    }
    while (!q.empty()) {
        int i = q.front();
        q.pop_front();
        VNode &n = nodes_[size_t(i)];
        n.visible = true;
        order.push_back(i);
        if (!n.collapsed)
            for (int c : n.children)
                q.push_back(c);
    }
    auto layerOf = [&](int i) { return std::clamp(nodes_[size_t(i)].layer, 0, 5); };
    // The children shown in the node's own layer.
    std::vector<std::vector<int>> kids(N);
    for (int i : order) {
        const VNode &n = nodes_[size_t(i)];
        if (n.collapsed)
            continue;
        for (int c : n.children)
            if (layerOf(c) == layerOf(i))
                kids[size_t(i)].push_back(c);
    }
    std::vector<int> gridIndex(N, -1), gridCols(N, 0);
    std::vector<double> width(N, 1);
    // A list of siblings: its leaves in a grid when there are many; the
    // width of the whole list, in slots.
    auto arrange = [&](const std::vector<int> &list) {
        std::vector<int> leaves;
        double w = 0;
        for (int c : list) {
            if (kids[size_t(c)].empty())
                leaves.push_back(c);
            else
                w += width[size_t(c)];
        }
        if (int(leaves.size()) > GridMin) {
            int cols = std::max(GridMin, int(std::ceil(std::sqrt(double(leaves.size()) * 2.5))));
            for (size_t k = 0; k < leaves.size(); k++) {
                gridIndex[size_t(leaves[k])] = int(k);
                gridCols[size_t(leaves[k])] = cols;
            }
            w += cols;
        } else {
            for (int c : leaves) {
                gridIndex[size_t(c)] = -1;
                w += 1;
            }
        }
        return w;
    };
    for (auto it = order.rbegin(); it != order.rend(); ++it)
        width[size_t(*it)] = std::max(1.0, arrange(kids[size_t(*it)]));
    // Place a list from column <start>, its first row <row>; then each
    // subtree under it.
    std::vector<int> maxRow(6, -1);
    std::function<void(const std::vector<int> &, double, int)> place = [&](const std::vector<int> &list, double start, int row) {
        arrange(list);      // (the grid of this list again: the indices are per list)
        double cur = start, gridStart = -1;
        for (int c : list) {
            VNode &cn = nodes_[size_t(c)];
            if (gridIndex[size_t(c)] >= 0) {
                if (gridStart < 0) {
                    gridStart = cur;
                    cur += gridCols[size_t(c)];
                }
                cn.x = gridStart + gridIndex[size_t(c)] % gridCols[size_t(c)] + 0.5;
                cn.row = row + gridIndex[size_t(c)] / gridCols[size_t(c)];
            } else if (kids[size_t(c)].empty()) {
                cn.x = cur + 0.5;
                cn.row = row;
                cur += 1;
            } else {
                cn.row = row;
                place(kids[size_t(c)], cur, row + 1);
                double lo = 1e18, hi = -1e18;
                for (int k : kids[size_t(c)]) {
                    lo = std::min(lo, nodes_[size_t(k)].x);
                    hi = std::max(hi, nodes_[size_t(k)].x);
                }
                cn.x = (lo + hi) / 2;
                cur += width[size_t(c)];
            }
            maxRow[size_t(layerOf(c))] = std::max(maxRow[size_t(layerOf(c))], cn.row);
        }
    };
    int nextRow = 0;
    for (int L = 0; L < 6; L++) {
        // The layer's roots, grouped by their parent above, in its order.
        std::vector<int> parents;
        QHash<int, std::vector<int>> groups;
        for (int i : order) {
            if (layerOf(i) != L)
                continue;
            int p = nodes_[size_t(i)].parent;
            if (p >= 0 && layerOf(p) == L)
                continue;
            if (!groups.contains(p))
                parents.push_back(p);
            groups[p].push_back(i);
        }
        if (parents.empty())
            continue;
        std::stable_sort(parents.begin(), parents.end(), [&](int a, int b) {
            double xa = a >= 0 ? nodes_[size_t(a)].x : -1e9, xb = b >= 0 ? nodes_[size_t(b)].x : -1e9;
            return xa < xb;
        });
        // Each group straight under its parent, as far as the groups
        // before it allow (left to right); then the whole layer moved back
        // by how far, on average, the groups had to give way.
        double cur = -1e18, drift = 0;
        int n = 0;
        for (int p : parents) {
            double w = arrange(groups[p]);
            double want = p >= 0 ? nodes_[size_t(p)].x - w / 2 : cur;
            double at = std::max(cur, want);
            if (cur == -1e18)
                at = p >= 0 ? want : 0;
            place(groups[p], at, nextRow);
            if (p >= 0) {
                drift += at - want;
                n++;
            }
            cur = at + w + 0.5;
        }
        if (n && drift != 0)
            for (int i : order)
                if (layerOf(i) == L)
                    nodes_[size_t(i)].x -= drift / n;
        nextRow = maxRow[size_t(L)] + 1;
    }
    // Bands and bounds.
    bands_.assign(6, {0, 0});
    bounds_ = QRectF();
    for (int i : order) {
        QRectF r = nodeRect(i);
        bounds_ = bounds_.isNull() ? r : bounds_.united(r);
        auto &b = bands_[size_t(std::clamp(nodes_[size_t(i)].layer, 0, 5))];
        if (b.first == 0 && b.second == 0)
            b = {r.top() - 16, r.bottom() + 16};
        else
            b = {std::min(b.first, r.top() - 16), std::max(b.second, r.bottom() + 16)};
    }
    // The heat scale: the largest share on screen.
    maxShare_ = 0;
    for (int i : order) {
        const VNode &n = nodes_[size_t(i)];
        if (!n.hw && n.kind != "idle")
            maxShare_ = std::max(maxShare_, share(n, n.collapsed));
    }
    emit layoutChanged();
}

QRectF GraphModel::nodeRect(int i) const
{
    const VNode &n = nodes_[size_t(i)];
    double y = n.row * (NodeH + VGap) + std::clamp(n.layer, 0, 5) * BandGap;
    return QRectF(n.x * slotW() - NodeW / 2, y, NodeW, NodeH);
}

int GraphModel::visibleOf(int i) const
{
    while (i >= 0 && !nodes_[size_t(i)].visible)
        i = nodes_[size_t(i)].parent;
    return i;
}

// ---------------------------------------------------------------- values
double GraphModel::total(int r) const
{
    return ofUsed_ ? used_[r] : totals_[r];
}

double GraphModel::share(const VNode &n, bool subtree) const
{
    if (n.hw || (ofUsed_ && n.kind == "idle"))
        return 0;
    double v = subtree ? n.sub[resource_] : n.res[resource_];
    double t = total(resource_);
    return t > 0 ? v / t : 0;
}

QString GraphModel::format(int r, double v) const
{
    switch (r) {
    case Cpu:
    case Gpu:
        return QString::number(v * 100, 'f', 1) + "%";
    case Mem:
    case Vram:
        return bytes(v);
    case Io:
    case Net:
        return rate(v);
    case Pwr:
        return QString::number(v, 'f', v < 10 ? 2 : 1) + " W";
    }
    return {};
}

QString GraphModel::deviceLoad(const VNode &n) const
{
    double v = n.res[resource_];
    if (v <= 0)
        return {};
    switch (resource_) {
    case Cpu:
    case Gpu:
        return "busy " + format(resource_, v);
    case Mem:
    case Vram:
        return format(resource_, v) + " used";
    default:
        return format(resource_, v);
    }
}

void GraphModel::computeTotals()
{
    QList<QVariantMap> l;
    auto add = [&](int r, const QString &text, const QString &detail) {
        l << QVariantMap{{"resource", r}, {"name", QString::fromLatin1(kResNames[r])}, {"summary", text}, {"detail", detail}};
    };
    add(Cpu, format(Cpu, used_[Cpu]) + " used", QString("of %1 CPUs").arg(cpus_));
    add(Mem, bytes(used_[Mem]) + " / " + bytes(totals_[Mem]), "used / total (free: \"idle / free\")");
    add(Gpu, gpus_ ? format(Gpu, used_[Gpu]) + " used" : "no GPU", QString("of %1 GPUs").arg(gpus_));
    add(Vram, totals_[Vram] > 0 ? bytes(used_[Vram]) + " / " + bytes(totals_[Vram]) : "none", "video memory");
    add(Io, rate(totals_[Io]), "all disks, read + written");
    add(Net, rate(totals_[Net]) + (netCaptured_ ? "" : " (no capture)"),
        netDrops_ > 0 ? QString("%1 packets not seen").arg(netDrops_) : "physical interfaces, in + out");
    add(Pwr, havePower_ ? format(Pwr, totals_[Pwr]) : "unknown",
        havePower_ ? QString("CPU %1, GPU %2 (estimate per process)").arg(format(Pwr, cpuW_), format(Pwr, gpuW_))
                   : "no energy counters readable");
    totals_m_->setRows(l);
}

void GraphModel::computeTop()
{
    std::vector<int> v;
    for (size_t i = 0; i < nodes_.size(); i++) {
        const VNode &n = nodes_[i];
        if (n.hw || n.res[resource_] <= 0 || (ofUsed_ && n.kind == "idle"))
            continue;
        v.push_back(int(i));
    }
    std::sort(v.begin(), v.end(), [&](int a, int b) { return nodes_[size_t(a)].res[resource_] > nodes_[size_t(b)].res[resource_]; });
    QList<QVariantMap> top;
    for (size_t k = 0; k < v.size() && k < 60; k++) {
        const VNode &n = nodes_[size_t(v[k])];
        top << QVariantMap{{"nodeId", n.id}, {"name", n.name}, {"pid", n.pid}, {"layerIndex", n.layer},
                           {"share", share(n)}, {"value", format(resource_, n.res[resource_])}};
    }
    top_->setRows(top);
}

void GraphModel::applySearch()
{
    matches_ = 0;
    QString s = search_.trimmed();
    for (auto &n : nodes_) {
        n.match = !s.isEmpty()
            && (n.name.contains(s, Qt::CaseInsensitive) || n.cmd.contains(s, Qt::CaseInsensitive)
                || (n.pid >= 0 && QString::number(n.pid) == s) || n.info.contains(s, Qt::CaseInsensitive));
        matches_ += n.match;
    }
}

QString GraphModel::nextMatch()
{
    std::vector<int> m;
    for (size_t i = 0; i < nodes_.size(); i++)
        if (nodes_[i].match)
            m.push_back(int(i));
    if (m.empty())
        return {};
    int i = m[size_t(matchCursor_++ % int(m.size()))];
    // Open what it is under, so it can be seen.
    bool opened = false;
    for (int p = nodes_[size_t(i)].parent; p >= 0; p = nodes_[size_t(p)].parent)
        if (collapsed_.remove(nodes_[size_t(p)].id)) {
            nodes_[size_t(p)].collapsed = false;
            opened = true;
        }
    if (opened)
        layout();
    select(nodes_[size_t(i)].id);
    return nodes_[size_t(i)].id;
}

// ---------------------------------------------------------------- interaction
void GraphModel::select(const QString &id)
{
    selectedKey_ = id;
    selected_ = index_.value(id, -1);
    computeSelected();
    emit selectionChanged();
    emit updated();
}

void GraphModel::toggleCollapse(const QString &id)
{
    int i = index_.value(id, -1);
    if (i < 0 || nodes_[size_t(i)].children.empty())
        return;
    VNode &n = nodes_[size_t(i)];
    n.collapsed = !n.collapsed;
    if (n.collapsed)
        collapsed_.insert(id);
    else
        collapsed_.remove(id);
    layout();
    computeSelected();
    emit updated();
    emit selectionChanged();
}

void GraphModel::setEdgeShown(const QString &type, bool on)
{
    if (on)
        hiddenEdges_.remove(type);
    else
        hiddenEdges_.insert(type);
    emit updated();
}

void GraphModel::computeSelected()
{
    if (selected_ < 0) {
        sel_->insert(QStringLiteral("id"), QString());
        selResources_->setRows({});
        selLinks_->setRows({});
        selMembers_->setRows({});
        return;
    }
    const VNode &n = nodes_[size_t(selected_)];
    QVariantHash m;
    m["id"] = n.id;
    m["name"] = n.name;
    m["kind"] = n.kind;
    m["layer"] = layerName(n.layer);
    m["pid"] = n.pid;
    m["user"] = n.user;
    m["cmd"] = n.cmd;
    m["state"] = n.state;
    m["info"] = n.info;
    m["threads"] = n.threads;
    m["fds"] = n.fds;
    m["hw"] = n.hw;
    m["irqs"] = n.irqs > 0 ? QString::number(n.irqs, 'f', 0) + " interrupts/s" : QString();
    m["children"] = int(n.children.size());
    m["collapsed"] = n.collapsed;
    m["parentId"] = n.parent >= 0 ? nodes_[size_t(n.parent)].id : QString();
    m["parentName"] = n.parent >= 0 ? nodes_[size_t(n.parent)].name : QString();
    QList<QVariantMap> res;
    for (int r = 0; r < ResourceCount; r++) {
        double t = total(r);
        QString share, sub;
        if (!n.hw && t > 0) {
            share = QString::number(n.res[r] / t * 100, 'f', 2) + "%";
            if (!n.children.empty())
                sub = QString::number(n.sub[r] / t * 100, 'f', 2) + "%";
        }
        res << QVariantMap{{"name", QString::fromLatin1(kResNames[r])}, {"value", format(r, n.res[r])}, {"share", share},
                           {"sub", sub}, {"current", r == resource_}};
    }
    selResources_->setRows(res);
    QList<QVariantMap> links;
    for (const auto &e : edges_) {
        if (e.a != selected_ && e.b != selected_)
            continue;
        int other = e.a == selected_ ? e.b : e.a;
        const VNode &o = nodes_[size_t(other)];
        bool directed = e.type == "pipe" || e.type == "dev" || e.type == "hw";
        QString dir = !directed ? "↔" : e.a == selected_ ? "→" : "←";
        links << QVariantMap{{"type", e.type}, {"nodeId", o.id},
                             {"name", o.name + (o.pid >= 0 ? QString(" (%1)").arg(o.pid) : QString())},
                             {"label", e.label}, {"dir", dir}, {"count", e.count}};
    }
    selLinks_->setRows(links);
    QList<QVariantMap> members;
    for (const auto &t : n.members)
        members << QVariantMap{{"line", t}};
    selMembers_->setRows(members);
    sel_->insert(m);
}
