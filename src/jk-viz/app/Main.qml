import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JkViz

ApplicationWindow {
    id: win
    width: 1600
    height: 940
    visible: true
    title: "jk-viz" + (graph.host ? " — " + graph.host : "")
    color: "#14181e"
    // Dark, whatever the desktop's colours (the graph is drawn on dark).
    palette.window: "#14181e"
    palette.windowText: "#e6e9ee"
    palette.base: "#0e1116"
    palette.alternateBase: "#191e26"
    palette.text: "#e6e9ee"
    palette.button: "#232a34"
    palette.buttonText: "#e6e9ee"
    palette.brightText: "#ffffff"
    palette.highlight: "#3d7fd6"
    palette.highlightedText: "#ffffff"
    palette.placeholderText: "#7d8694"
    palette.mid: "#2c3440"
    palette.midlight: "#323b48"
    palette.light: "#3a4452"
    palette.dark: "#0b0d11"
    palette.shadow: "#000000"
    palette.toolTipBase: "#191e26"
    palette.toolTipText: "#e6e9ee"

    GraphModel {
        id: graph
        objectName: "graph"
    }

    readonly property var edgeTypes: [
        { t: "tree",   n: "parent → child", c: "#7d8694" },
        { t: "socket", n: "Unix sockets",   c: "#5aa7f0" },
        { t: "dbus",   n: "D-Bus",          c: "#c67be0" },
        { t: "pipe",   n: "pipes",          c: "#8fd16a" },
        { t: "tcp",    n: "TCP (local)",    c: "#e8c25c" },
        { t: "net",    n: "network",        c: "#f0954a" },
        { t: "dev",    n: "devices",        c: "#a9b4c4" },
        { t: "hw",     n: "drivers",        c: "#e5737d" }
    ]

    header: ToolBar {
        background: Rectangle { color: "#1a1f27" }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 4
            Label {
                text: "jk-viz"
                font.pixelSize: 17
                font.bold: true
                rightPadding: 12
            }
            Repeater {
                model: ["CPU", "Memory", "GPU", "VRAM", "Disk I/O", "Network", "Power"]
                delegate: Button {
                    required property string modelData
                    required property int index
                    text: modelData
                    checkable: true
                    checked: graph.resource === index
                    flat: !checked
                    onClicked: graph.resource = index
                }
            }
            ToolSeparator {}
            Button {
                text: graph.ofUsed ? "% of what's used" : "% of the total"
                flat: true
                ToolTip.visible: hovered
                ToolTip.text: "Shares of the whole resource (idle and free included, so everything adds up to 100%),\nor of only what is in use."
                onClicked: graph.ofUsed = !graph.ofUsed
            }
            Item { Layout.fillWidth: true }
            TextField {
                id: search
                Layout.preferredWidth: 260
                placeholderText: "Find: name, command, pid  (Enter: next)"
                onTextChanged: graph.search = text
                onAccepted: {
                    var id = graph.nextMatch()
                    if (id)
                        view.centerOnNode(id)
                }
            }
            Label {
                text: graph.search ? graph.matches + " found" : ""
                color: "#ffd84a"
                Layout.preferredWidth: 64
            }
            Button { text: "Home"; flat: true; onClicked: view.home() }
            Button { text: "Fit"; flat: true; onClicked: view.fit() }
            Button { text: "−"; flat: true; onClicked: view.zoomBy(1 / 1.3) }
            Button { text: "+"; flat: true; onClicked: view.zoomBy(1.3) }
            Button {
                text: graph.paused ? "Resume" : "Pause"
                highlighted: graph.paused
                onClicked: graph.paused = !graph.paused
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        GraphView {
            id: view
            objectName: "view"
            model: graph
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            Label {
                anchors.centerIn: parent
                visible: graph.top.count === 0
                text: graph.status
                font.pixelSize: 15
                color: "#8b94a3"
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: "#2a313c"
        }
        SidePanel {
            graph: graph
            view: view
            Layout.fillHeight: true
            Layout.preferredWidth: 400
        }
    }

    footer: ToolBar {
        background: Rectangle { color: "#1a1f27" }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 2
            Repeater {
                model: win.edgeTypes
                delegate: Button {
                    required property var modelData
                    flat: true
                    checkable: true
                    checked: true
                    onToggled: graph.setEdgeShown(modelData.t, checked)
                    contentItem: Row {
                        spacing: 6
                        Rectangle {
                            width: 16; height: 3; radius: 1.5
                            anchors.verticalCenter: parent.verticalCenter
                            color: modelData.c
                            opacity: parent.parent.checked ? 1 : 0.25
                        }
                        Label {
                            text: modelData.n
                            opacity: parent.parent.checked ? 1 : 0.45
                        }
                    }
                }
            }
            Item { Layout.fillWidth: true }
            Label {
                text: "click: details · double-click / right-click: fold · drag: move · wheel: zoom"
                color: "#7d8694"
                rightPadding: 16
            }
            Label {
                text: graph.status
                color: graph.status === "connected" ? "#8fcf6e" : "#e8c25c"
            }
        }
    }
}
