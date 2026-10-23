import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JkUpdateGui

ApplicationWindow {
    id: win
    width: 720
    height: 560
    visible: true
    title: "Software Update"
    color: "#14181e"
    palette.window: "#14181e"
    palette.windowText: "#e6e9ee"
    palette.base: "#0e1116"
    palette.text: "#e6e9ee"
    palette.button: "#232a34"
    palette.buttonText: "#e6e9ee"
    palette.highlight: "#3d7fd6"
    palette.highlightedText: "#ffffff"

    // jk-update gui --install: the notification's Install button.
    property bool installAtStart: false

    readonly property color dimColor: "#aeb7c4"
    readonly property color goodColor: "#8bd6a3"
    readonly property color newColor: "#7fb6f0"
    readonly property color warnColor: "#ef818a"

    Updater { id: up }
    Component.onCompleted: Qt.callLater(function() { installAtStart ? up.install() : up.check() })

    function mb(bytes) { return Math.max(1, Math.round(bytes / 1048576)) + " MB" }
    // "jk_os 0.2.0", or for another build of the version running now (a
    // re-publish) "a new build of jk_os 0.1.0".
    readonly property bool sameVersion: up.newVersion !== "" && up.newVersion === up.installedVersion
    readonly property string release: (sameVersion ? "a new build of jk_os " : "jk_os ") + up.newVersion
    function upper(text) { return text.charAt(0).toUpperCase() + text.slice(1) }

    readonly property string headline: {
        switch (up.state) {
        case "checking":    return "Checking for updates…"
        case "uptodate":    return "jk_os is up to date"
        case "available":   return upper(release) + " is available"
        case "downloading": return "Downloading " + release + "…"
        case "installing":  return "Installing " + release + "…"
        case "installed":
        case "pending":     return upper(release) + " is installed"
        case "error":       return "The update didn't go through"
        default:            return "Software Update"
        }
    }
    readonly property string detail: {
        switch (up.state) {
        case "available":
            return (up.totalBytes > 0 ? mb(up.totalBytes) + " to download. " : "")
                 + "It installs in the background; you keep working, and restart when it's done."
        case "downloading":
            return up.totalBytes > 0 ? mb(up.doneBytes) + " of " + mb(up.totalBytes) : up.message
        case "installing":  return "Putting the new image and kernel in place; don't switch off."
        case "installed":
        case "pending":     return "It starts at the next restart. The release you run now stays for sudo jk-update --rollback."
        case "error":       return up.error
        default:            return up.message
        }
    }
    readonly property color stateColor: up.state === "error" ? warnColor
                                      : up.state === "uptodate" || up.state === "installed" || up.state === "pending" ? goodColor
                                      : newColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Label { text: "Software Update"; font.pixelSize: 28; font.bold: true }
        Label {
            text: "Installed: jk_os " + (up.installedVersion || "?")
                  + (up.installedBuild ? " (build " + up.installedBuild + ")" : "")
                  + (up.channel ? "   ·   channel " + up.channel : "")
            color: dimColor
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: card.implicitHeight + 32
            radius: 10
            color: "#1b212a"
            border.color: "#303947"

            ColumnLayout {
                id: card
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    spacing: 10
                    BusyIndicator {
                        running: up.busy
                        visible: up.busy
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                    }
                    Label { text: win.headline; font.pixelSize: 20; font.bold: true; color: win.stateColor }
                }
                Label {
                    text: win.detail
                    color: dimColor
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
                ProgressBar {
                    Layout.fillWidth: true
                    visible: up.state === "downloading" || up.state === "installing"
                    indeterminate: up.state === "installing" || up.totalBytes <= 0
                    value: up.totalBytes > 0 ? up.doneBytes / up.totalBytes : 0
                }
                Label {
                    visible: up.releaseDate !== "" && (up.state === "available" || up.state === "downloading")
                    text: "Published " + up.releaseDate + (up.newBuild ? "   ·   build " + up.newBuild : "")
                    color: "#8a93a1"
                    font.pixelSize: 12
                }

                RowLayout {
                    spacing: 10
                    Button {
                        text: "Install"
                        visible: up.state === "available"
                        enabled: !up.busy
                        highlighted: true
                        onClicked: up.install()
                    }
                    Button {
                        text: "Restart now"
                        visible: up.state === "installed" || up.state === "pending"
                        highlighted: true
                        onClicked: up.restart()
                    }
                    Button {
                        text: "Later"
                        visible: up.state === "installed" || up.state === "pending"
                        onClicked: win.close()
                    }
                    Button {
                        text: up.state === "error" ? "Try again" : "Check again"
                        visible: !up.busy && up.state !== "installed" && up.state !== "pending"
                        onClicked: up.check()
                    }
                }
            }
        }

        // Files you changed that the new release changes too: your copies
        // stay in use (jk-update's list).
        Label {
            visible: up.shadowed.length > 0
            text: "These files you changed are also updated; your copies stay in use:\n    "
                  + up.shadowed.join("\n    ")
                  + "\nTo take the new one: sudo rm /data/system/root/upper<path>, then restart."
            color: "#e8c46a"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Label { text: "Details"; color: dimColor; visible: up.log.length > 0 }
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: up.log.length > 0
            TextArea {
                readOnly: true
                text: up.log.join("\n")
                font.family: "monospace"
                font.pixelSize: 12
                color: "#c9d1dc"
                background: Rectangle { color: "#0e1116"; radius: 6 }
            }
        }
        Item { Layout.fillHeight: true; visible: up.log.length === 0 }
    }
}
