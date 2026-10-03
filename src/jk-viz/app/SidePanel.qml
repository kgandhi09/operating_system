import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JkViz

// The totals of each resource, the selected node, and who uses the most.
Rectangle {
    id: panel
    required property GraphModel graph
    required property GraphView view
    color: "#14181e"

    readonly property QtObject sel: graph.selected
    readonly property bool hasSel: graph.selectedId !== ""
    readonly property var layerColors: ["#d8a83a", "#b48ae0", "#e5737d", "#62a8ec", "#4fc1c9", "#8fcf6e"]
    readonly property var edgeColors: ({ socket: "#5aa7f0", dbus: "#c67be0", pipe: "#8fd16a", tcp: "#e8c25c",
                                         net: "#f0954a", dev: "#a9b4c4", hw: "#e5737d" })

    function pick(id) {
        graph.select(id)
        view.centerOnNode(id)
    }

    component Heading: Label {
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 1.2
        color: "#8b94a3"
        topPadding: 14
        bottomPadding: 4
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: panel.width - 28
            x: 14
            spacing: 2

            // ------------------------------------------------ totals
            Heading { text: "100% OF EACH RESOURCE" }
            Repeater {
                model: panel.graph.totals
                delegate: Rectangle {
                    id: total
                    required property int resource
                    required property string name
                    required property string summary
                    required property string detail
                    Layout.fillWidth: true
                    implicitHeight: 40
                    radius: 6
                    color: panel.graph.resource === resource ? "#243042" : (ma.containsMouse ? "#1c222b" : "transparent")
                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: panel.graph.resource = total.resource
                    }
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 0
                        RowLayout {
                            Label { text: total.name; font.bold: true; Layout.fillWidth: true }
                            Label { text: total.summary }
                        }
                        Label {
                            text: total.detail
                            color: "#7d8694"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // ------------------------------------------------ selection
            Heading { text: "SELECTED"; visible: panel.hasSel }
            ColumnLayout {
                visible: panel.hasSel
                Layout.fillWidth: true
                spacing: 3
                Label {
                    text: panel.sel.name ?? ""
                    font.pixelSize: 18
                    font.bold: true
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Label {
                    text: (panel.sel.layer ?? "") + " · " + (panel.sel.kind ?? "")
                        + (panel.sel.pid >= 0 ? " · pid " + panel.sel.pid : "")
                        + (panel.sel.user ? " · " + panel.sel.user : "")
                        + (panel.sel.state ? " · state " + panel.sel.state : "")
                    color: "#8b94a3"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Label {
                    visible: (panel.sel.threads ?? 0) > 0 || (panel.sel.fds ?? 0) > 0
                    text: (panel.sel.threads ? panel.sel.threads + " threads" : "")
                        + (panel.sel.fds ? " · " + panel.sel.fds + " open files" : "")
                    color: "#8b94a3"
                }
                TextEdit {
                    visible: !!panel.sel.cmd
                    text: panel.sel.cmd ?? ""
                    readOnly: true
                    selectByMouse: true
                    wrapMode: Text.WrapAnywhere
                    color: "#c9d1db"
                    font.family: "monospace"
                    font.pixelSize: 11
                    Layout.fillWidth: true
                }
                Label {
                    visible: !!panel.sel.info
                    text: panel.sel.info ?? ""
                    color: "#a9b4c4"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                Label {
                    visible: !!panel.sel.irqs
                    text: panel.sel.irqs ?? ""
                    color: "#a9b4c4"
                }
                Label {
                    visible: !!panel.sel.parentName
                    text: "under <a href='#'>" + (panel.sel.parentName ?? "") + "</a>"
                    color: "#8b94a3"
                    linkColor: "#62a8ec"
                    onLinkActivated: panel.pick(panel.sel.parentId)
                }
                // Resources: the node's amount, its share of 100%, and with
                // everything under it.
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 6
                    spacing: 12
                    Item { Layout.preferredWidth: 80 }
                    Label {
                        text: panel.sel.hw ? "device" : "amount"
                        color: "#7d8694"; font.pixelSize: 11
                        Layout.fillWidth: true; horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: panel.sel.hw ? "" : "share"
                        color: "#7d8694"; font.pixelSize: 11
                        Layout.preferredWidth: 62; horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: panel.sel.hw || !panel.sel.children ? "" : "with below"
                        color: "#7d8694"; font.pixelSize: 11
                        Layout.preferredWidth: 72; horizontalAlignment: Text.AlignRight
                    }
                }
                Repeater {
                    model: panel.graph.selResources
                    delegate: RowLayout {
                        id: resRow
                        required property string name
                        required property string value
                        required property string share
                        required property string sub
                        required property bool current
                        Layout.fillWidth: true
                        spacing: 12
                        Label {
                            text: resRow.name
                            font.bold: resRow.current
                            color: resRow.current ? "#ffffff" : "#a9b4c4"
                            Layout.preferredWidth: 80
                        }
                        Label {
                            text: resRow.value
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                        Label {
                            text: resRow.share
                            Layout.preferredWidth: 62
                            horizontalAlignment: Text.AlignRight
                            font.bold: resRow.current
                        }
                        Label {
                            text: resRow.sub
                            color: "#8b94a3"
                            Layout.preferredWidth: 72
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
                Button {
                    visible: (panel.sel.children ?? 0) > 0
                    text: (panel.sel.collapsed ? "Unfold (" : "Fold (") + panel.sel.children + " below)"
                    flat: true
                    onClicked: panel.graph.toggleCollapse(panel.sel.id)
                }
                Heading {
                    visible: panel.graph.selMembers.count > 0
                    text: "THREADS"
                }
                Repeater {
                    model: panel.graph.selMembers
                    delegate: Label {
                        required property string line
                        text: line
                        font.pixelSize: 11
                        color: "#c9d1db"
                    }
                }
                Heading {
                    visible: panel.graph.selLinks.count > 0
                    text: "CONNECTED TO (" + panel.graph.selLinks.count + ")"
                }
                Repeater {
                    model: panel.graph.selLinks
                    delegate: Rectangle {
                        id: link
                        required property string type
                        required property string nodeId
                        required property string name
                        required property string label
                        required property string dir
                        required property int count
                        Layout.fillWidth: true
                        implicitHeight: lc.implicitHeight + 6
                        radius: 4
                        color: lma.containsMouse ? "#1c222b" : "transparent"
                        MouseArea {
                            id: lma
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: panel.pick(link.nodeId)
                        }
                        ColumnLayout {
                            id: lc
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 0
                            Label {
                                text: link.dir + " " + link.name + (link.count > 1 ? "  ×" + link.count : "")
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: (link.type === "dbus" ? "D-Bus" : link.type) + (link.label ? ": " + link.label : "")
                                color: panel.edgeColors[link.type] ?? "#a9b4c4"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

            // ------------------------------------------------ top
            Heading {
                text: "TOP · " + ["CPU", "MEMORY", "GPU", "VRAM", "DISK I/O", "NETWORK", "POWER"][panel.graph.resource]
                    + (panel.graph.ofUsed ? " (OF WHAT'S USED)" : " (OF 100%)")
            }
            Repeater {
                model: panel.graph.top
                delegate: Rectangle {
                    id: row
                    required property string nodeId
                    required property string name
                    required property int pid
                    required property int layerIndex
                    required property double share
                    required property string value
                    Layout.fillWidth: true
                    implicitHeight: 30
                    radius: 4
                    color: panel.graph.selectedId === nodeId ? "#243042" : (tma.containsMouse ? "#1c222b" : "transparent")
                    MouseArea {
                        id: tma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: panel.pick(row.nodeId)
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                        height: 3
                        radius: 1.5
                        width: Math.max(2, parent.width * Math.min(1, row.share))
                        color: panel.layerColors[row.layerIndex] ?? "#8b94a3"
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        spacing: 8
                        Label {
                            text: row.name
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: row.pid >= 0 ? row.pid : ""
                            color: "#7d8694"
                            font.pixelSize: 11
                        }
                        Label {
                            text: panel.graph.resource === 0 || panel.graph.resource === 2 ? "" : row.value
                            color: "#a9b4c4"
                        }
                        Label {
                            text: (row.share * 100).toFixed(row.share < 0.1 ? 2 : 1) + "%"
                            font.bold: true
                            Layout.preferredWidth: 56
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
            Item { implicitHeight: 16 }
        }
    }
}
