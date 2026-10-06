import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JkPowerGui

ApplicationWindow {
    id: win
    width: 1040
    height: 760
    visible: true
    title: "Power Settings"
    color: "#14181e"
    palette.window: "#14181e"
    palette.windowText: "#e6e9ee"
    palette.base: "#0e1116"
    palette.text: "#e6e9ee"
    palette.button: "#232a34"
    palette.buttonText: "#e6e9ee"
    palette.highlight: "#3d7fd6"
    palette.highlightedText: "#ffffff"

    CliSettings {
        id: controls
        Component.onCompleted: refresh()
    }

    function run(args) {
        if (controls.invoke(args, password.text))
            password.clear()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label { text: "Power Settings"; font.pixelSize: 28; font.bold: true }
        Label {
            text: "Only controls provided by this machine appear below. Changes are saved by jk-power."
            color: "#aeb7c4"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Label { text: "Mode" }
            ComboBox {
                id: modeChoice
                model: controls.modes
                Layout.preferredWidth: 190
                currentIndex: controls.modes.indexOf(controls.mode)
            }
            Button { text: "Use mode"; onClicked: win.run([modeChoice.currentText]) }
            Button { text: "Reset"; onClicked: win.run(["reset"]) }
            TextField {
                id: modeName
                placeholderText: "New mode name"
                color: "#e6e9ee"
                placeholderTextColor: "#8a93a1"
                Layout.preferredWidth: 175
            }
            Button { text: "Save mode"; enabled: modeName.text.length > 0; onClicked: win.run(["save", modeName.text]) }
            Button {
                text: "Delete mode"
                enabled: modeChoice.currentText !== "quiet" && modeChoice.currentText !== "balanced"
                         && modeChoice.currentText !== "performance"
                onClicked: win.run(["delete", modeChoice.currentText])
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            TextField {
                id: password
                echoMode: TextInput.Password
                placeholderText: "Administrator password for changes"
                color: "#e6e9ee"
                placeholderTextColor: "#8a93a1"
                Layout.preferredWidth: 320
            }
            TextField {
                id: search
                placeholderText: "Find a setting"
                color: "#e6e9ee"
                placeholderTextColor: "#8a93a1"
                Layout.fillWidth: true
            }
            Button { text: "Refresh"; onClicked: controls.refresh() }
        }

        Label {
            visible: controls.error !== ""
            text: controls.error
            color: "#ef818a"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            visible: controls.error === "" && controls.message !== ""
            text: controls.message.split("\n")[0]
            color: "#8bd6a3"
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        ListView {
            id: settingsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: controls.rows.filter(function(row) {
                const query = search.text.toLowerCase()
                return query === "" || (row.key + " " + row.group + " " + row.label).toLowerCase().includes(query)
            })
            ScrollBar.vertical: ScrollBar { }
            delegate: Rectangle {
                required property var modelData
                readonly property bool selectable: ["cpugov", "idle", "online", "noturbo", "boost",
                                                    "epp", "dfgov", "amdlevel", "profile", "nvpers",
                                                    "nvboost"].includes(modelData.type)
                width: settingsView.width - 12
                height: 76
                radius: 8
                color: "#1b212a"
                border.color: "#303947"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 12
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: modelData.key; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                        Label {
                            text: modelData.group + " · " + modelData.label + " · " + modelData.choices
                            color: "#aeb7c4"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    Label {
                        text: modelData.current + (modelData.unit ? " " + modelData.unit : "")
                              + "  (" + modelData.source + ")"
                        Layout.preferredWidth: 165
                        elide: Text.ElideRight
                    }
                    TextField {
                        id: value
                        visible: !selectable
                        text: modelData.current
                        color: "#e6e9ee"
                        Layout.preferredWidth: 130
                        selectByMouse: true
                    }
                    ComboBox {
                        id: choice
                        visible: selectable
                        model: selectable ? modelData.choices.trim().split(/\s+/) : []
                        currentIndex: model.indexOf(modelData.current)
                        Layout.preferredWidth: 130
                    }
                    Button {
                        text: "Apply"
                        onClicked: win.run(["set", modelData.key + "=" + (selectable ? choice.currentText : value.text)])
                    }
                    Button { text: "Unset"; onClicked: win.run(["unset", modelData.key]) }
                }
            }
        }
    }
}
