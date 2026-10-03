// jk-vizd: a snapshot as one line of JSON (what jk-viz reads).
#include "model.h"
#include "util.h"

namespace jkv {

std::string toJson(const Snapshot &s)
{
    Json j;
    j.out.reserve(256 * 1024);
    j.beginObj();
    j.kv("t", s.t).kv("interval", s.interval).kv("host", s.host);
    const Totals &t = s.totals;
    j.key("totals").beginObj();
    j.kv("cpus", t.cpus).kv("mem", t.memTotal).kv("gpus", t.gpus).kv("vram", t.vramTotal);
    j.kv("io", t.io).kv("net", t.net).kv("netCaptured", t.haveNet).kv("netDrops", t.netDrops);
    j.kv("cpuW", t.cpuWatts).kv("gpuW", t.gpuWatts).kv("power", t.havePower);
    j.endObj();
    j.key("nodes").beginArr();
    for (const Node &n : s.nodes) {
        j.beginObj();
        j.kv("id", n.id).kv("kind", n.kind).kv("layer", n.layer).kv("name", n.name);
        if (!n.parent.empty())
            j.kv("parent", n.parent);
        if (n.hw)
            j.kv("hw", true);
        if (n.pid >= 0)
            j.kv("pid", n.pid);
        if (!n.user.empty())
            j.kv("user", n.user);
        if (!n.cmd.empty())
            j.kv("cmd", n.cmd.size() > 400 ? n.cmd.substr(0, 400) + "…" : n.cmd);
        if (!n.state.empty())
            j.kv("state", n.state);
        if (!n.info.empty())
            j.kv("info", n.info);
        if (n.threads)
            j.kv("threads", n.threads);
        if (n.fds)
            j.kv("fds", n.fds);
        if (n.irqs > 0)
            j.kv("irqs", n.irqs);
        const Res &r = n.r;
        if (r.cpu > 0 || r.mem || r.gpu > 0 || r.vram || r.io > 0 || r.net > 0 || r.pwr > 0) {
            j.key("r").beginObj();
            if (r.cpu > 0)
                j.kv("cpu", r.cpu);
            if (r.mem)
                j.kv("mem", r.mem);
            if (r.gpu > 0)
                j.kv("gpu", r.gpu);
            if (r.vram)
                j.kv("vram", r.vram);
            if (r.io > 0)
                j.kv("io", r.io);
            if (r.net > 0)
                j.kv("net", r.net);
            if (r.pwr > 0)
                j.kv("pwr", r.pwr);
            j.endObj();
        }
        if (!n.members.empty()) {
            j.key("members").beginArr();
            for (auto &m : n.members)
                j.val(m);
            j.endArr();
        }
        j.endObj();
    }
    j.endArr();
    j.key("edges").beginArr();
    for (const Edge &e : s.edges) {
        j.beginObj();
        j.kv("a", e.a).kv("b", e.b).kv("type", e.type);
        if (!e.label.empty())
            j.kv("label", e.label);
        if (e.count > 1)
            j.kv("n", e.count);
        j.endObj();
    }
    j.endArr();
    j.endObj();
    j.out += '\n';
    return std::move(j.out);
}

} // namespace jkv
