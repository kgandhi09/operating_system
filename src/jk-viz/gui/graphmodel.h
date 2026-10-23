// jk-viz: the graph of one snapshot, laid out in layers (silicon at the
// top, applications at the bottom), and what the side panel shows.
#pragma once
#include <QAbstractListModel>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSet>
#include <QVariant>
#include <QQmlPropertyMap>
#include <QtQml/qqmlregistration.h>
#include <vector>

class VizClient;

// Rows for the side panel's lists (QML reads them as a model, not as
// arrays of objects copied out on every update).
class RowsModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit RowsModel(const QList<QByteArray> &roles, QObject *parent = nullptr);
    int rowCount(const QModelIndex & = {}) const override { return int(rows_.size()); }
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return int(rows_.size()); }
    void setRows(const QList<QVariantMap> &rows);
signals:
    void countChanged();
private:
    QList<QByteArray> roles_;
    QList<QVariantMap> rows_;
};

enum Resource { Cpu, Mem, Gpu, Vram, Io, Net, Pwr, ResourceCount };

struct VNode {
    QString id, kind, name, parentId, user, cmd, state, info;
    QStringList members;
    int layer = 0, pid = -1, threads = 0, fds = 0;
    bool hw = false;
    double irqs = 0;
    double res[ResourceCount] = {};
    double sub[ResourceCount] = {};     // with everything below it
    // Layout
    int parent = -1;
    std::vector<int> children;
    bool visible = true, collapsed = false, match = false;
    int row = 0;
    double x = 0;                       // centre, in slots
    int depth = 0;
};

struct VEdge {
    int a = -1, b = -1;
    QString type, label;
    int count = 1;
};

class GraphModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int resource READ resource WRITE setResource NOTIFY resourceChanged)
    Q_PROPERTY(bool ofUsed READ ofUsed WRITE setOfUsed NOTIFY resourceChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(QString search READ search WRITE setSearch NOTIFY searchChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString host READ host NOTIFY updated)
    Q_PROPERTY(QString selectedId READ selectedId NOTIFY selectionChanged)
    // The selected node (its fields, and its lists below).
    Q_PROPERTY(QQmlPropertyMap *selected READ selected CONSTANT)
    Q_PROPERTY(RowsModel *selResources READ selResources CONSTANT)
    Q_PROPERTY(RowsModel *selLinks READ selLinks CONSTANT)
    Q_PROPERTY(RowsModel *selMembers READ selMembers CONSTANT)
    Q_PROPERTY(RowsModel *top READ top CONSTANT)
    Q_PROPERTY(RowsModel *totals READ totals CONSTANT)
    Q_PROPERTY(QStringList resourceNames READ resourceNames CONSTANT)
    Q_PROPERTY(int matches READ matches NOTIFY searchChanged)

public:
    explicit GraphModel(QObject *parent = nullptr);
    ~GraphModel() override;

    // Layout geometry (scene units).
    static constexpr double NodeW = 176, NodeH = 48, HGap = 14, VGap = 30, BandGap = 46;
    static double slotW() { return NodeW + HGap; }

    int resource() const { return resource_; }
    void setResource(int r);
    bool ofUsed() const { return ofUsed_; }
    void setOfUsed(bool b);
    bool paused() const { return paused_; }
    void setPaused(bool b);
    QString search() const { return search_; }
    void setSearch(const QString &s);
    QString status() const;
    QString host() const { return host_; }
    QString selectedId() const { return selected_ >= 0 ? nodes_[size_t(selected_)].id : QString(); }
    QQmlPropertyMap *selected() const { return sel_; }
    RowsModel *selResources() const { return selResources_; }
    RowsModel *selLinks() const { return selLinks_; }
    RowsModel *selMembers() const { return selMembers_; }
    RowsModel *top() const { return top_; }
    RowsModel *totals() const { return totals_m_; }
    QStringList resourceNames() const;
    int matches() const { return matches_; }

    Q_INVOKABLE void select(const QString &id);
    Q_INVOKABLE void toggleCollapse(const QString &id);
    Q_INVOKABLE void setEdgeShown(const QString &type, bool on);
    Q_INVOKABLE bool edgeShown(const QString &type) const { return !hiddenEdges_.contains(type); }
    Q_INVOKABLE QString format(int resource, double value) const;
    Q_INVOKABLE QString nextMatch();
    // Load a snapshot from a file instead (tests, screenshots).
    Q_INVOKABLE void loadFile(const QString &path);

    // For the view.
    const std::vector<VNode> &nodes() const { return nodes_; }
    const std::vector<VEdge> &edges() const { return edges_; }
    int selectedIndex() const { return selected_; }
    int indexOf(const QString &id) const { return index_.value(id, -1); }
    QRectF nodeRect(int i) const;
    QRectF bounds() const { return bounds_; }
    // Each layer's band: top and bottom (scene y), or empty.
    std::vector<std::pair<double, double>> bands() const { return bands_; }
    static QString layerName(int l);
    // The node's share of the selected resource (of the total, or of what's used), 0..1.
    double share(const VNode &n, bool subtree = false) const;
    double total(int r) const;
    // A device's own load of the selected resource, as text.
    QString deviceLoad(const VNode &n) const;
    double maxShare() const { return maxShare_; }
    int visibleOf(int i) const;     // the node, or its nearest visible ancestor

signals:
    void resourceChanged();
    void pausedChanged();
    void searchChanged();
    void statusChanged();
    void selectionChanged();
    void updated();
    void layoutChanged();
    void centerOn(double x, double y);

private:
    VizClient *client_;
    std::vector<VNode> nodes_;
    std::vector<VEdge> edges_;
    QHash<QString, int> index_;
    QSet<QString> collapsed_, hiddenEdges_;
    QJsonObject last_;
    QString host_, search_, selectedKey_;
    int selected_ = -1, resource_ = Cpu, matches_ = 0, matchCursor_ = 0;
    bool ofUsed_ = false, paused_ = false;
    QQmlPropertyMap *sel_;
    RowsModel *selResources_, *selLinks_, *selMembers_, *top_, *totals_m_;
    QRectF bounds_;
    std::vector<std::pair<double, double>> bands_ = std::vector<std::pair<double, double>>(6, {0.0, 0.0});
    double totals_[ResourceCount] = {}, used_[ResourceCount] = {};
    int cpus_ = 0, gpus_ = 0;
    double cpuW_ = 0, gpuW_ = 0, netDrops_ = 0, interval_ = 0;
    bool havePower_ = false, netCaptured_ = false;
    double maxShare_ = 0;

    void ingest(const QJsonObject &s);
    void rebuild();
    void layout();
    void computeTop();
    void computeTotals();
    void computeSelected();
    void applySearch();
};
