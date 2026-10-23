// jk-link gui: the mesh console.
//   header     identity of this machine, the mesh's numbers, visibility
//   topology   MeshView: the machines and the links between them
//   inspector  the selected machine: identity, network, trust, actions;
//              pairing requests on top
//   event log  what jk-linkd did lately
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import JkLink

Window {
    id: win
    width: 1440
    height: 900
    minimumWidth: 1080
    minimumHeight: 680
    visible: true
    title: "jk-link — Mesh Network Console"
    color: c.bg0

    // The palette (MeshView uses the same).
    QtObject {
        id: c
        readonly property color bg0: "#05080c"
        readonly property color bg1: "#070b10"
        readonly property color panel: "#0b1118"
        readonly property color panel2: "#0f1720"
        readonly property color line: "#1a2632"
        readonly property color line2: "#243341"
        readonly property color text: "#e6edf3"
        readonly property color dim: "#7d8a99"
        readonly property color faint: "#4b5866"
        readonly property color teal: "#1fd1c1"
        readonly property color green: "#3de07a"
        readonly property color amber: "#f5a524"
        readonly property color blue: "#4c8dff"
        readonly property color red: "#ff4d5e"
        readonly property color slate: "#4a5562"
        readonly property string ui: "Inter"
        readonly property string mono: "FiraCode Nerd Font Mono"
    }

    LinkClient {
        id: link
        onCommandDone: (cmd, ok, error, answer) => {
            if (!ok) {
                toast.show(error)
                if (cmd === "pair")
                    pairDialog.close()
                return
            }
            if (cmd === "pair") {
                pairDialog.openOutgoing(answer.id, answer.name, answer.code)
            } else if (cmd === "accept") {
                toast.show("Paired with " + pairDialog.peerName, true)
                pairDialog.close()
            } else if (cmd === "reject") {
                pairDialog.close()
            } else if (cmd === "unpair") {
                toast.show("Unpaired", true)
            }
        }
    }

    // jk-link gui <machine>: the machine selected first.
    property string initialSelect
    onStChanged: {
        if (!initialSelect || !peers.length)
            return
        const p = peers.find(p => p.name === initialSelect || p.id === initialSelect || p.id.startsWith(initialSelect))
        if (p)
            mesh.selected = p.key
        initialSelect = ""
    }

    readonly property var st: link.status
    readonly property var me: st.self || ({})
    readonly property var peers: st.peers || []
    readonly property var requests: st.requests || []
    readonly property var pairings: st.pairings || []
    readonly property var events: st.events || []

    // The selected machine, as one object: this machine, a known one, or one
    // only seen through another's links.
    readonly property var sel: {
        const k = mesh.selected
        if (!k || k === me.key)
            return Object.assign({ isSelf: true, online: true, paired: false, link: "up" }, me)
        for (let i = 0; i < peers.length; ++i)
            if (peers[i].key === k)
                return peers[i]
        for (let i = 0; i < peers.length; ++i)
            for (const n of (peers[i].neighbors || []))
                if (n.key === k)
                    return { key: k, id: n.id, name: "", remote: true, via: peers[i].name, online: true }
        return Object.assign({ isSelf: true, online: true }, me)
    }
    readonly property var selRequest: requests.find(r => r.key === sel.key)
    readonly property var selPairing: pairings.find(p => p.key === sel.key)

    function rttText(r) {
        if (r === undefined || r === null || r < 0) return "—"
        if (r < 1) return r.toFixed(2) + " ms"
        if (r < 10) return r.toFixed(1) + " ms"
        return Math.round(r) + " ms"
    }
    function duration(s) {
        if (s === undefined || s === null) return "—"
        if (s < 60) return Math.floor(s) + " s"
        if (s < 3600) return Math.floor(s / 60) + " min " + Math.floor(s % 60) + " s"
        if (s < 86400) return Math.floor(s / 3600) + " h " + Math.floor((s % 3600) / 60) + " min"
        return Math.floor(s / 86400) + " d " + Math.floor((s % 86400) / 3600) + " h"
    }
    function stateText(p) {
        if (p.isSelf) return "THIS NODE"
        if (p.remote) return "VIA " + (p.via || "MESH").toUpperCase()
        if (p.paired) return p.online ? (p.link === "up" ? "LINKED" : "CONNECTING") : "OFFLINE"
        return p.online ? "DISCOVERED" : "GONE"
    }
    function stateColor(p) {
        if (p.isSelf) return c.teal
        if (p.remote) return c.teal
        if (p.paired) return p.online ? (p.link === "up" ? c.green : c.amber) : c.slate
        return p.online ? c.blue : c.slate
    }
    readonly property int nodesOnline: 1 + peers.filter(p => p.online).length
    readonly property int pairedCount: peers.filter(p => p.paired).length
    readonly property int linksUp: peers.filter(p => p.link === "up").length
    readonly property string medianRtt: {
        const r = peers.filter(p => p.link === "up" && p.rtt >= 0).map(p => p.rtt).sort((a, b) => a - b)
        return r.length ? rttText(r[Math.floor(r.length / 2)]) : "—"
    }

    // ---------------------------------------------------------- components
    component Label: Text {
        color: c.text
        font.family: c.ui
        font.pixelSize: 13
        renderType: Text.NativeRendering
    }
    component Mono: Text {
        color: c.text
        font.family: c.mono
        font.pixelSize: 12
    }
    component Caption: Text {
        color: c.dim
        font.family: c.ui
        font.pixelSize: 10
        font.weight: Font.DemiBold
        font.letterSpacing: 1.4
        font.capitalization: Font.AllUppercase
    }
    component Btn: Rectangle {
        id: b
        property string text
        property string kind: "normal"   // normal, primary, danger, ghost
        property bool enabledState: true
        signal clicked
        implicitWidth: Math.max(88, t.implicitWidth + 28)
        implicitHeight: 32
        radius: 3
        opacity: enabledState ? 1 : 0.4
        color: kind === "primary" ? (ma.pressed ? "#17a89b" : ma.containsMouse ? "#25e0cf" : c.teal)
             : kind === "danger" ? (ma.containsMouse ? "#3a1418" : "#2a1014")
             : (ma.containsMouse ? c.panel2 : "transparent")
        border.width: kind === "primary" ? 0 : 1
        border.color: kind === "danger" ? c.red : ma.containsMouse ? c.line2 : c.line
        Text {
            id: t
            anchors.centerIn: parent
            text: b.text
            color: b.kind === "primary" ? "#03100f" : b.kind === "danger" ? c.red : c.text
            font.family: c.ui
            font.pixelSize: 12
            font.weight: Font.DemiBold
            font.letterSpacing: 0.6
        }
        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            enabled: b.enabledState
            onClicked: b.clicked()
        }
    }
    component Led: Rectangle {
        property color tint: c.green
        property bool blink: false
        width: 8
        height: 8
        radius: 4
        color: tint
        Rectangle {
            anchors.centerIn: parent
            width: 16
            height: 16
            radius: 8
            color: "transparent"
            border.color: parent.tint
            opacity: 0.35
        }
        SequentialAnimation on opacity {
            running: blink
            loops: Animation.Infinite
            NumberAnimation { to: 0.25; duration: 600 }
            NumberAnimation { to: 1; duration: 600 }
        }
    }
    component Kpi: Item {
        property string caption
        property string value
        property string unit
        property color tint: c.text
        implicitWidth: Math.max(96, col.implicitWidth + 28)
        implicitHeight: 56
        Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.margins: 12; width: 1; color: c.line }
        Column {
            id: col
            anchors.verticalCenter: parent.verticalCenter
            x: 22
            spacing: 2
            Caption { text: caption }
            Row {
                spacing: 4
                Text { text: value; color: tint; font.family: c.mono; font.pixelSize: 19; font.weight: Font.Medium }
                Text { text: unit; color: c.dim; font.family: c.mono; font.pixelSize: 11; anchors.baseline: parent.children[0].baseline }
            }
        }
    }
    component Section: Column {
        property string title
        default property alias content: body.data
        width: parent ? parent.width : 0
        spacing: 8
        Row {
            spacing: 8
            width: parent.width
            Rectangle { width: 3; height: 12; color: c.teal; anchors.verticalCenter: parent.verticalCenter }
            Caption { text: title; anchors.verticalCenter: parent.verticalCenter }
        }
        Column {
            id: body
            width: parent.width
            spacing: 6
        }
    }
    component Field: Item {
        property string key
        property string value
        property bool mono: true
        property color tint: c.text
        width: parent ? parent.width : 0
        implicitHeight: Math.max(k.implicitHeight, v.implicitHeight)
        Text {
            id: k
            text: key
            width: 104
            color: c.dim
            font.family: c.ui
            font.pixelSize: 11
            font.letterSpacing: 0.4
        }
        Text {
            id: v
            anchors.left: k.right
            anchors.right: parent.right
            text: value
            color: tint
            wrapMode: Text.WrapAnywhere
            font.family: mono ? c.mono : c.ui
            font.pixelSize: 12
        }
    }
    component Segmented: Row {
        id: seg
        property var options: []
        property string current
        signal picked(string value)
        spacing: 0
        Repeater {
            model: seg.options
            Rectangle {
                required property string modelData
                required property int index
                width: lbl.implicitWidth + 22
                height: 26
                color: modelData === seg.current ? Qt.rgba(0.12, 0.82, 0.76, 0.16) : (sm.containsMouse ? c.panel2 : "transparent")
                border.color: modelData === seg.current ? c.teal : c.line2
                border.width: 1
                Text {
                    id: lbl
                    anchors.centerIn: parent
                    text: modelData.toUpperCase()
                    color: modelData === seg.current ? c.teal : c.dim
                    font.family: c.ui
                    font.pixelSize: 10
                    font.weight: Font.Bold
                    font.letterSpacing: 1.2
                }
                MouseArea {
                    id: sm
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: seg.picked(modelData)
                }
            }
        }
    }
    component HexIcon: Canvas {
        property color tint: c.teal
        property bool filled: false
        width: 22
        height: 24
        onTintChanged: requestPaint()
        onPaint: {
            const g = getContext("2d")
            g.reset()
            const cx = width / 2, cy = height / 2, r = Math.min(width, height) / 2 - 1.5
            g.beginPath()
            for (let i = 0; i < 6; ++i) {
                const a = (60 * i - 90) * Math.PI / 180
                const x = cx + r * Math.cos(a), y = cy + r * Math.sin(a)
                i ? g.lineTo(x, y) : g.moveTo(x, y)
            }
            g.closePath()
            g.lineWidth = 1.8
            g.strokeStyle = tint
            g.stroke()
            g.beginPath()
            g.arc(cx, cy, r * 0.28, 0, 2 * Math.PI)
            g.fillStyle = tint
            g.fill()
        }
    }

    // ------------------------------------------------------------- header
    Rectangle {
        id: header
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: 60
        color: c.bg1
        Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: c.line }

        Row {
            id: brand
            anchors.verticalCenter: parent.verticalCenter
            x: 18
            spacing: 12
            HexIcon { width: 30; height: 32; anchors.verticalCenter: parent.verticalCenter }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1
                Text { text: "JK·LINK"; color: c.text; font.family: c.ui; font.pixelSize: 17; font.weight: Font.Bold; font.letterSpacing: 2.5 }
                Caption { text: "Mesh network console"; font.pixelSize: 9 }
            }
        }

        Row {
            anchors { left: brand.right; leftMargin: 26; top: parent.top; bottom: parent.bottom }
            Kpi { caption: "Nodes online"; value: nodesOnline; unit: "/ " + (peers.length + 1) }
            Kpi { caption: "Paired"; value: pairedCount; tint: c.teal }
            Kpi { caption: "Links up"; value: linksUp; unit: "/ " + pairedCount; tint: linksUp === pairedCount ? c.green : c.amber }
            Kpi { caption: "Median RTT"; value: medianRtt.replace(" ms", ""); unit: medianRtt === "—" ? "" : "ms" }
            Kpi { caption: "Requests"; value: requests.length; tint: requests.length ? c.amber : c.dim }
            Item {
                implicitWidth: visCol.implicitWidth + 34
                height: parent.height
                Rectangle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.margins: 12; width: 1; color: c.line }
                Column {
                    id: visCol
                    x: 22
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 5
                    Caption { text: "Visibility" }
                    Segmented {
                        options: ["open", "paired", "off"]
                        current: me.visibility || ""
                        onPicked: v => link.command("visibility", { mode: v })
                    }
                }
            }
        }

        Row {
            anchors { right: parent.right; rightMargin: 18; verticalCenter: parent.verticalCenter }
            spacing: 18
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3
                Row {
                    spacing: 8
                    anchors.right: parent.right
                    Led { tint: link.connected ? c.green : c.red; blink: !link.connected; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: link.connected ? "JK-LINKD ONLINE" : "JK-LINKD OFFLINE"; color: link.connected ? c.text : c.red; font.family: c.ui; font.pixelSize: 11; font.weight: Font.Bold; font.letterSpacing: 1.2 }
                }
                Text {
                    anchors.right: parent.right
                    text: (me.name || "—") + "  ·  " + (me.id || "")
                    color: c.dim
                    font.family: c.mono
                    font.pixelSize: 11
                }
            }
            Rectangle { width: 1; height: 34; color: c.line; anchors.verticalCenter: parent.verticalCenter }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text { id: clock; color: c.text; font.family: c.mono; font.pixelSize: 17 }
                Text { id: date; color: c.dim; font.family: c.mono; font.pixelSize: 10 }
                Timer {
                    interval: 1000; running: true; repeat: true; triggeredOnStart: true
                    onTriggered: {
                        const d = new Date()
                        clock.text = Qt.formatTime(d, "HH:mm:ss")
                        date.text = Qt.formatDate(d, "yyyy-MM-dd")
                    }
                }
            }
        }
    }

    // ----------------------------------------------------------- footer
    Rectangle {
        id: footer
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 24
        color: c.bg1
        Rectangle { anchors { left: parent.left; right: parent.right; top: parent.top } height: 1; color: c.line }
        Row {
            anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
            spacing: 22
            Mono { text: "UDP 7710 · TCP " + (me.port || 7710) + " · TLS 1.3 · Ed25519"; color: c.faint; font.pixelSize: 10 }
            Mono { text: "FP " + (me.fingerprint || "—"); color: c.faint; font.pixelSize: 10 }
        }
        Mono {
            anchors { right: parent.right; rightMargin: 14; verticalCenter: parent.verticalCenter }
            text: (me.version || "") + " · " + (me.arch || "")
            color: c.faint
            font.pixelSize: 10
        }
    }

    // ---------------------------------------------------------- event log
    Rectangle {
        id: logPane
        property bool open: true
        anchors { left: parent.left; right: inspector.left; bottom: footer.top }
        height: open ? 178 : 32
        color: c.bg1
        Behavior on height { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
        Rectangle { anchors { left: parent.left; right: parent.right; top: parent.top } height: 1; color: c.line }
        Item {
            id: logHead
            anchors { left: parent.left; right: parent.right; top: parent.top }
            height: 32
            Row {
                anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
                spacing: 10
                Rectangle { width: 3; height: 12; color: c.teal; anchors.verticalCenter: parent.verticalCenter }
                Caption { text: "Event log"; anchors.verticalCenter: parent.verticalCenter }
                Mono { text: events.length + " entries"; color: c.faint; font.pixelSize: 10; anchors.verticalCenter: parent.verticalCenter }
            }
            Text {
                anchors { right: parent.right; rightMargin: 14; verticalCenter: parent.verticalCenter }
                text: logPane.open ? "▾  COLLAPSE" : "▴  EXPAND"
                color: c.dim
                font.family: c.ui
                font.pixelSize: 10
                font.weight: Font.Bold
                font.letterSpacing: 1.2
                MouseArea { anchors.fill: parent; anchors.margins: -6; cursorShape: Qt.PointingHandCursor; onClicked: logPane.open = !logPane.open }
            }
        }
        ListView {
            anchors { left: parent.left; right: parent.right; top: logHead.bottom; bottom: parent.bottom; leftMargin: 14; rightMargin: 14; bottomMargin: 6 }
            clip: true
            visible: logPane.open
            model: events.slice().reverse()
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: Row {
                required property var modelData
                spacing: 14
                height: 21
                Mono { text: Qt.formatDateTime(new Date(modelData.time * 1000), "HH:mm:ss"); color: c.faint; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
                Rectangle {
                    width: 62; height: 15; radius: 2
                    anchors.verticalCenter: parent.verticalCenter
                    readonly property color lc: modelData.level === "warn" ? c.amber : modelData.level === "error" ? c.red : modelData.level === "request" ? c.blue : c.teal
                    color: Qt.rgba(lc.r, lc.g, lc.b, 0.12)
                    border.color: Qt.rgba(lc.r, lc.g, lc.b, 0.5)
                    Text { anchors.centerIn: parent; text: modelData.level.toUpperCase(); color: parent.lc; font.family: c.ui; font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 1 }
                }
                Text { text: modelData.text; color: c.text; font.family: c.ui; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            }
        }
    }

    // ------------------------------------------------------------ topology
    Item {
        id: canvas
        anchors { left: parent.left; right: inspector.left; top: header.bottom; bottom: logPane.top }
        clip: true

        MeshView {
            id: mesh
            anchors.fill: parent
            status: win.st
            fitTop: 56
            fitBottom: 48
        }

        // Toolbar.
        Row {
            anchors { left: parent.left; top: parent.top; margins: 14 }
            spacing: 6
            Rectangle {
                height: 30; width: tag.implicitWidth + 24; radius: 3
                color: Qt.rgba(0.04, 0.07, 0.09, 0.85); border.color: c.line
                Row {
                    id: tag
                    anchors.centerIn: parent
                    spacing: 8
                    Led { tint: link.connected ? c.teal : c.red; blink: true; width: 6; height: 6; anchors.verticalCenter: parent.verticalCenter }
                    Caption { text: "Topology · live"; color: c.text; anchors.verticalCenter: parent.verticalCenter }
                }
            }
            Item { width: 6; height: 1 }
            Repeater {
                model: [
                    { t: "FIT", f: () => mesh.fit() },
                    { t: "−", f: () => mesh.zoomBy(1 / 1.2) },
                    { t: "+", f: () => mesh.zoomBy(1.2) },
                    { t: "RE-LAYOUT", f: () => mesh.relayout() }
                ]
                Rectangle {
                    required property var modelData
                    height: 30; width: Math.max(30, tt.implicitWidth + 20); radius: 3
                    color: tm.containsMouse ? c.panel2 : Qt.rgba(0.04, 0.07, 0.09, 0.85)
                    border.color: tm.containsMouse ? c.line2 : c.line
                    Text { id: tt; anchors.centerIn: parent; text: modelData.t; color: c.text; font.family: c.ui; font.pixelSize: modelData.t.length > 1 ? 10 : 15; font.weight: Font.Bold; font.letterSpacing: 1.2 }
                    MouseArea { id: tm; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: modelData.f() }
                }
            }
            Item { width: 6; height: 1 }
            Repeater {
                model: [ { t: "OFFLINE", p: "showOffline" }, { t: "UNPAIRED", p: "showUnpaired" } ]
                Rectangle {
                    required property var modelData
                    readonly property bool on: mesh[modelData.p]
                    height: 30; width: ft.implicitWidth + 34; radius: 3
                    color: on ? Qt.rgba(0.12, 0.82, 0.76, 0.10) : Qt.rgba(0.04, 0.07, 0.09, 0.85)
                    border.color: on ? Qt.rgba(0.12, 0.82, 0.76, 0.5) : c.line
                    Rectangle { x: 10; anchors.verticalCenter: parent.verticalCenter; width: 8; height: 8; radius: 1; color: on ? c.teal : "transparent"; border.color: on ? c.teal : c.dim }
                    Text { id: ft; x: 24; anchors.verticalCenter: parent.verticalCenter; text: modelData.t; color: on ? c.text : c.dim; font.family: c.ui; font.pixelSize: 10; font.weight: Font.Bold; font.letterSpacing: 1.2 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: mesh[modelData.p] = !mesh[modelData.p] }
                }
            }
        }

        // Legend: a chip that opens.
        Rectangle {
            id: legendBox
            property bool open: false
            anchors { left: parent.left; bottom: parent.bottom; margins: 14 }
            width: open ? legend.implicitWidth + 24 : legendChip.implicitWidth + 24
            height: open ? legend.implicitHeight + 20 : 30
            radius: 3
            clip: true
            color: Qt.rgba(0.04, 0.07, 0.09, 0.88)
            border.color: c.line
            Caption {
                id: legendChip
                visible: !legendBox.open
                anchors.centerIn: parent
                text: "▸  Legend"
            }
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: legendBox.open = !legendBox.open }
            Column {
                id: legend
                visible: legendBox.open
                anchors.centerIn: parent
                spacing: 6
                Caption { text: "▾  Legend" }
                Repeater {
                    model: [
                        { k: "line", col: c.teal, t: "Encrypted link · round-trip time" },
                        { k: "dash", col: c.amber, t: "Paired · connecting" },
                        { k: "dot", col: c.blue, t: "Discovered · not paired" },
                        { k: "hex", col: c.green, t: "Link up" },
                        { k: "hex", col: c.slate, t: "Offline" },
                        { k: "hex", col: c.amber, t: "Pairing / request" }
                    ]
                    Row {
                        id: legendRow
                        required property var modelData
                        spacing: 10
                        Item {
                            width: 26; height: 12
                            anchors.verticalCenter: parent.verticalCenter
                            Rectangle { visible: modelData.k === "line"; anchors.verticalCenter: parent.verticalCenter; width: 26; height: 2; color: modelData.col }
                            Row {
                                visible: modelData.k === "dash" || modelData.k === "dot"
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 3
                                Repeater { model: legendRow.modelData.k === "dot" ? 6 : 4; Rectangle { width: legendRow.modelData.k === "dot" ? 2 : 4; height: 2; color: legendRow.modelData.col } }
                            }
                            HexIcon { visible: modelData.k === "hex"; tint: modelData.col; width: 12; height: 13; anchors.centerIn: parent }
                        }
                        Text { text: modelData.t; color: c.dim; font.family: c.ui; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
                    }
                }
            }
        }

        // Zoom readout.
        Mono {
            anchors { right: parent.right; bottom: parent.bottom; margins: 14 }
            text: "ZOOM " + Math.round(mesh.zoom * 100) + "%  ·  DRAG TO PAN  ·  WHEEL TO ZOOM  ·  DOUBLE-CLICK TO FIT"
            color: c.faint
            font.pixelSize: 10
        }

        // No daemon.
        Rectangle {
            anchors.centerIn: parent
            visible: !link.connected
            width: 440; height: 120; radius: 4
            color: c.panel
            border.color: c.red
            Column {
                anchors.centerIn: parent
                spacing: 8
                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    Led { tint: c.red; blink: true; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: "NO CONNECTION TO JK-LINKD"; color: c.red; font.family: c.ui; font.pixelSize: 13; font.weight: Font.Bold; font.letterSpacing: 1.5 }
                }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: link.error; color: c.dim; font.family: c.mono; font.pixelSize: 11 }
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Start it with:  sudo /etc/init.d/S45jk-link start"; color: c.text; font.family: c.mono; font.pixelSize: 11 }
            }
        }
    }

    // ----------------------------------------------------------- inspector
    Rectangle {
        id: inspector
        anchors { right: parent.right; top: header.bottom; bottom: footer.top }
        width: 392
        color: c.panel
        Rectangle { anchors { left: parent.left; top: parent.top; bottom: parent.bottom } width: 1; color: c.line }

        Flickable {
            id: scroll
            anchors.fill: parent
            anchors.leftMargin: 1
            contentHeight: body.implicitHeight + 40
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            Column {
                id: body
                x: 20
                y: 18
                width: inspector.width - 40
                spacing: 20

                // Pairing requests, always first.
                Repeater {
                    model: requests
                    Rectangle {
                        required property var modelData
                        width: body.width
                        height: rq.implicitHeight + 28
                        radius: 4
                        color: Qt.rgba(0.96, 0.65, 0.14, 0.07)
                        border.color: c.amber
                        Column {
                            id: rq
                            x: 14; y: 14
                            width: parent.width - 28
                            spacing: 10
                            Row {
                                spacing: 8
                                Led { tint: c.amber; blink: true; anchors.verticalCenter: parent.verticalCenter }
                                Caption { text: "Incoming pairing request"; color: c.amber; anchors.verticalCenter: parent.verticalCenter }
                            }
                            Label { text: modelData.name || modelData.id; font.pixelSize: 16; font.weight: Font.DemiBold }
                            Mono { text: modelData.id; color: c.dim; font.pixelSize: 11 }
                            Row {
                                spacing: 8
                                Btn { text: "Review & accept"; kind: "primary"; onClicked: pairDialog.openIncoming(modelData.id, modelData.name, modelData.code) }
                                Btn { text: "Reject"; kind: "danger"; onClicked: link.command("reject", { peer: modelData.id }) }
                            }
                        }
                    }
                }

                // The selected machine.
                Row {
                    spacing: 14
                    width: body.width
                    HexIcon { width: 40; height: 44; tint: stateColor(sel); anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 56
                        spacing: 4
                        Text { text: sel.name || sel.id || "—"; color: c.text; font.family: c.ui; font.pixelSize: 21; font.weight: Font.DemiBold; elide: Text.ElideRight; width: parent.width }
                        Row {
                            spacing: 8
                            Rectangle {
                                height: 18; width: pillText.implicitWidth + 16; radius: 2
                                readonly property color pc: stateColor(sel)
                                color: Qt.rgba(pc.r, pc.g, pc.b, 0.14)
                                border.color: Qt.rgba(pc.r, pc.g, pc.b, 0.6)
                                Text { id: pillText; anchors.centerIn: parent; text: stateText(sel); color: parent.pc; font.family: c.ui; font.pixelSize: 9; font.weight: Font.Bold; font.letterSpacing: 1.3 }
                            }
                            Mono { text: sel.id || ""; color: c.dim; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }
                }

                Rectangle { width: body.width; height: 1; color: c.line }

                Section {
                    title: "Identity"
                    Field { key: "Machine ID"; value: sel.id || "—" }
                    Field { key: "Fingerprint"; value: sel.fingerprint || (sel.remote ? "known to " + sel.via + " only" : "—"); tint: c.dim }
                    Field { key: "System"; value: sel.version || "—"; visible: !sel.remote }
                    Field { key: "Architecture"; value: sel.arch || "—"; visible: !sel.remote }
                }

                Section {
                    title: "Network"
                    visible: !sel.remote
                    Field {
                        key: "Addresses"
                        value: (sel.isSelf ? (sel.addresses || []) : (sel.addresses || [])).join("\n") || "—"
                    }
                    Field { key: sel.isSelf ? "Listening" : "Port"; value: sel.isSelf ? "TCP " + (sel.port || 7710) + " · UDP 7710" : ((sel.addresses || [])[0] || "—").split(":").pop() }
                    Field { key: "Link"; value: sel.isSelf ? (linksUp + " up") : (sel.link || "—").toUpperCase(); tint: sel.link === "up" ? c.green : sel.link === "connecting" ? c.amber : c.text; visible: !sel.isSelf }
                    Field { key: "Round trip"; value: rttText(sel.rtt); visible: !sel.isSelf && sel.paired }
                    Field { key: "Link uptime"; value: duration(sel.link_since); visible: !sel.isSelf && sel.link === "up" }
                    Field { key: "Last seen"; value: sel.last_seen === null || sel.last_seen === undefined ? "never" : duration(sel.last_seen) + " ago"; visible: !sel.isSelf }
                }

                Section {
                    title: "Trust"
                    visible: !sel.isSelf && !sel.remote
                    Field { key: "Status"; value: sel.paired ? "Paired · encrypted, key verified" : "Not paired · no data exchanged"; mono: false; tint: sel.paired ? c.green : c.dim }
                    Field { key: "Paired since"; value: sel.paired_since ? Qt.formatDateTime(new Date(sel.paired_since * 1000), "yyyy-MM-dd HH:mm") : "—"; visible: !!sel.paired }
                    Field { key: "Name then"; value: sel.paired_name || "—"; visible: !!sel.paired }
                    Rectangle {
                        visible: !!sel.renamed
                        width: parent.width; height: rn.implicitHeight + 16; radius: 3
                        color: Qt.rgba(0.96, 0.65, 0.14, 0.08); border.color: c.amber
                        Text { id: rn; x: 10; y: 8; width: parent.width - 20; wrapMode: Text.Wrap; color: c.amber; font.family: c.ui; font.pixelSize: 11
                               text: "Paired as “" + (sel.paired_name || "") + "”, now calls itself “" + (sel.name || "") + "”. The key is the same: it is the same machine." }
                    }
                }

                Section {
                    title: "Mesh links"
                    visible: !sel.isSelf && !!sel.paired
                    Repeater {
                        model: sel.neighbors || []
                        Field { required property var modelData; key: "→ " + ((peers.find(p => p.key === modelData.key) || {}).name || modelData.id); value: rttText(modelData.rtt); mono: true }
                    }
                    Text { visible: !(sel.neighbors || []).length; text: "No other links reported"; color: c.faint; font.family: c.ui; font.pixelSize: 11 }
                }

                // This machine: what can be set.
                Section {
                    title: "This node"
                    visible: !!sel.isSelf
                    Text {
                        width: parent.width
                        wrapMode: Text.Wrap
                        color: c.dim
                        font.family: c.ui
                        font.pixelSize: 11
                        text: me.visibility === "open" ? "Announced on every network it is on. Any jk_os machine can see it and ask to pair."
                            : me.visibility === "paired" ? "Announced only to the machines it is paired with. Invisible to the rest."
                            : "Not announced at all. Paired machines can still reach it at the addresses they know."
                    }
                    Item { width: 1; height: 4 }
                    Caption { text: "Reach a machine where multicast doesn't" }
                    Row {
                        spacing: 8
                        width: parent.width
                        TextField {
                            id: addField
                            width: parent.width - addBtn.width - 8
                            height: 32
                            placeholderText: "192.168.1.42  or  [fe80::1%eth0]:7710"
                            placeholderTextColor: c.faint
                            color: c.text
                            font.family: c.mono
                            font.pixelSize: 12
                            leftPadding: 10
                            background: Rectangle { color: c.bg1; border.color: addField.activeFocus ? c.teal : c.line2; radius: 3 }
                            onAccepted: addBtn.clicked()
                        }
                        Btn { id: addBtn; text: "Add"; onClicked: { if (addField.text.trim()) { link.command("add", { address: addField.text.trim() }); addField.text = "" } } }
                    }
                    Repeater {
                        model: st.manual || []
                        Row {
                            required property var modelData
                            spacing: 8
                            Mono { text: modelData; anchors.verticalCenter: parent.verticalCenter; width: body.width - 90 }
                            Btn { text: "Remove"; kind: "ghost"; implicitWidth: 72; onClicked: link.command("remove", { address: modelData }) }
                        }
                    }
                }

                // Actions on another machine.
                Column {
                    width: body.width
                    spacing: 10
                    visible: !sel.isSelf && !sel.remote
                    Rectangle { width: body.width; height: 1; color: c.line }
                    Row {
                        spacing: 8
                        Btn {
                            text: "Pair with this node"
                            kind: "primary"
                            visible: !sel.paired && !selRequest
                            enabledState: !!sel.online && !selPairing
                            onClicked: { pairDialog.openConnecting(sel.id, sel.name); link.command("pair", { peer: sel.id }) }
                        }
                        Btn {
                            text: "Review request"
                            kind: "primary"
                            visible: !!selRequest
                            onClicked: pairDialog.openIncoming(selRequest.id, selRequest.name, selRequest.code)
                        }
                        Btn {
                            text: "Unpair"
                            kind: "danger"
                            visible: !!sel.paired
                            onClicked: unpairDialog.ask(sel.id, sel.name)
                        }
                    }
                    Text {
                        width: body.width
                        wrapMode: Text.Wrap
                        color: c.faint
                        font.family: c.ui
                        font.pixelSize: 11
                        text: sel.paired ? "Unpairing makes both machines forget each other; pairing again needs both users."
                                         : "Pairing shows a code on both machines. Compare them: the same code on both means nobody is in between."
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------- dialogs
    Rectangle {
        id: scrim
        anchors.fill: parent
        color: Qt.rgba(0.01, 0.02, 0.03, 0.72)
        visible: pairDialog.visible || unpairDialog.visible
        MouseArea { anchors.fill: parent }
    }

    Rectangle {
        id: pairDialog
        property string mode: "out"     // out (this machine asked) or in (a request)
        property string peerId
        property string peerName
        property string code
        property string phase: "code"   // connecting, code, waiting, done, failed
        property string failure
        visible: false
        anchors.centerIn: parent
        width: 520
        height: dlg.implicitHeight + 48
        radius: 5
        color: c.panel
        border.color: c.line2

        function openConnecting(id, name) { mode = "out"; peerId = id; peerName = name || id; code = ""; phase = "connecting"; visible = true }
        function openOutgoing(id, name, cd) { mode = "out"; peerId = id; peerName = name || id; code = cd; phase = "code"; visible = true }
        function openIncoming(id, name, cd) { mode = "in"; peerId = id; peerName = name || id; code = cd; phase = "code"; visible = true }
        function close() { visible = false }

        // Follow the pairing this machine started.
        readonly property var live: pairings.find(p => p.id === peerId)
        onLiveChanged: {
            if (!visible || mode !== "out" || !live || phase === "code" || phase === "connecting")
                return
            if (live.state === "paired") phase = "done"
            else if (live.state !== "waiting" && live.state !== "connecting") { phase = "failed"; failure = live.state }
        }

        Column {
            id: dlg
            x: 24; y: 24
            width: parent.width - 48
            spacing: 16
            Row {
                spacing: 10
                HexIcon { width: 22; height: 24; tint: pairDialog.phase === "failed" ? c.red : pairDialog.phase === "done" ? c.green : c.amber; anchors.verticalCenter: parent.verticalCenter }
                Caption { text: pairDialog.mode === "in" ? "Incoming pairing request" : "Pairing"; color: c.text; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            }
            Column {
                spacing: 3
                Text { text: pairDialog.peerName; color: c.text; font.family: c.ui; font.pixelSize: 22; font.weight: Font.DemiBold }
                Mono { text: pairDialog.peerId; color: c.dim; font.pixelSize: 11 }
            }

            // The code, digit by digit.
            Row {
                visible: pairDialog.code !== ""
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 8
                Repeater {
                    model: pairDialog.code.split("")
                    Rectangle {
                        required property string modelData
                        width: modelData === " " ? 14 : 52
                        height: 66
                        radius: 4
                        color: modelData === " " ? "transparent" : c.bg1
                        border.color: modelData === " " ? "transparent" : c.line2
                        Text { anchors.centerIn: parent; text: modelData; color: c.teal; font.family: c.mono; font.pixelSize: 36; font.weight: Font.Medium }
                    }
                }
            }

            Text {
                width: parent.width
                wrapMode: Text.Wrap
                color: c.dim
                font.family: c.ui
                font.pixelSize: 12
                lineHeight: 1.25
                text: pairDialog.phase === "connecting" ? "Connecting to " + pairDialog.peerName + " and checking its key…"
                    : pairDialog.phase === "waiting" ? "Codes confirmed here. Waiting for " + pairDialog.peerName + " to accept the request…"
                    : pairDialog.phase === "done" ? "Paired. The two machines now trust each other's keys and keep an encrypted link."
                    : pairDialog.phase === "failed" ? "Not paired: " + pairDialog.failure + "."
                    : pairDialog.mode === "in"
                      ? "Does " + pairDialog.peerName + " show this same code? Only accept if it does: a different code means something between the two machines is pretending to be one of them."
                      : pairDialog.peerName + " now shows a pairing request with a code. Confirm here only if it is this same code; it must be accepted there too."
            }

            Rectangle {
                visible: pairDialog.phase === "waiting" || pairDialog.phase === "connecting"
                width: parent.width; height: 3; color: c.line; clip: true
                Rectangle {
                    width: parent.width * 0.3; height: 3; color: c.amber
                    SequentialAnimation on x {
                        running: pairDialog.visible; loops: Animation.Infinite
                        NumberAnimation { from: -pairDialog.width * 0.3; to: pairDialog.width; duration: 1300; easing.type: Easing.InOutQuad }
                    }
                }
            }

            Row {
                anchors.right: parent.right
                spacing: 8
                Btn {
                    text: pairDialog.mode === "in" ? "Reject" : "Codes differ"
                    kind: "danger"
                    visible: pairDialog.phase === "code"
                    onClicked: {
                        if (pairDialog.mode === "in") link.command("reject", { peer: pairDialog.peerId })
                        else link.command("confirm", { peer: pairDialog.peerId, yes: false })
                        pairDialog.close()
                    }
                }
                Btn {
                    text: pairDialog.mode === "in" ? "Codes match · accept" : "Codes match"
                    kind: "primary"
                    visible: pairDialog.phase === "code"
                    onClicked: {
                        if (pairDialog.mode === "in") link.command("accept", { peer: pairDialog.peerId })
                        else { link.command("confirm", { peer: pairDialog.peerId, yes: true }); pairDialog.phase = "waiting" }
                    }
                }
                Btn {
                    text: pairDialog.phase === "done" || pairDialog.phase === "failed" ? "Close" : "Cancel"
                    kind: "ghost"
                    visible: pairDialog.phase !== "code"
                    onClicked: {
                        if (pairDialog.phase === "waiting" || pairDialog.phase === "connecting")
                            link.command("confirm", { peer: pairDialog.peerId, yes: false })
                        pairDialog.close()
                    }
                }
            }
        }
    }

    Rectangle {
        id: unpairDialog
        property string peerId
        property string peerName
        function ask(id, name) { peerId = id; peerName = name || id; visible = true }
        visible: false
        anchors.centerIn: parent
        width: 440
        height: ud.implicitHeight + 48
        radius: 5
        color: c.panel
        border.color: c.red
        Column {
            id: ud
            x: 24; y: 24
            width: parent.width - 48
            spacing: 14
            Caption { text: "Unpair"; color: c.red; font.pixelSize: 11 }
            Text { text: "Unpair " + unpairDialog.peerName + "?"; color: c.text; font.family: c.ui; font.pixelSize: 18; font.weight: Font.DemiBold }
            Text { width: parent.width; wrapMode: Text.Wrap; color: c.dim; font.family: c.ui; font.pixelSize: 12
                   text: "Both machines forget each other and the link closes. Pairing again needs the code confirmed on both." }
            Row {
                anchors.right: parent.right
                spacing: 8
                Btn { text: "Keep"; kind: "ghost"; onClicked: unpairDialog.visible = false }
                Btn { text: "Unpair"; kind: "danger"; onClicked: { link.command("unpair", { peer: unpairDialog.peerId }); unpairDialog.visible = false } }
            }
        }
    }

    // Messages (errors from jk-linkd, confirmations).
    Rectangle {
        id: toast
        property bool good: false
        function show(text, isGood) { msg.text = text; good = !!isGood; opacity = 1; hide.restart() }
        anchors { horizontalCenter: canvas.horizontalCenter; bottom: logPane.top; bottomMargin: 18 }
        width: msg.implicitWidth + 36
        height: 38
        radius: 4
        opacity: 0
        visible: opacity > 0
        color: c.panel2
        border.color: good ? c.green : c.red
        Behavior on opacity { NumberAnimation { duration: 180 } }
        Text { id: msg; anchors.centerIn: parent; color: toast.good ? c.green : c.text; font.family: c.ui; font.pixelSize: 12 }
        Timer { id: hide; interval: 4200; onTriggered: toast.opacity = 0 }
    }
}
