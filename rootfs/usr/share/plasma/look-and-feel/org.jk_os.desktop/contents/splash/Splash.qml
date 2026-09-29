/*
    jk_os's splash screen, shown while Plasma starts: the J.K. Robotics logo
    on the wallpaper's light background. Based on Breeze's (Marco Martin,
    GPL-2.0-or-later); ksplashqml advances `stage` as startup progresses.
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

Rectangle {
    id: root
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#f6f8f7" }
        GradientStop { position: 1.0; color: "#e2e8e5" }
    }

    property int stage

    onStageChanged: {
        if (stage == 2) {
            introAnimation.running = true;
        } else if (stage == 5) {
            introAnimation.target = busyIndicator;
            introAnimation.from = 1;
            introAnimation.to = 0;
            introAnimation.running = true;
        }
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0

        Image {
            id: logo
            readonly property real size: Kirigami.Units.gridUnit * 14

            anchors.centerIn: parent
            anchors.verticalCenterOffset: -Kirigami.Units.gridUnit

            asynchronous: true
            source: "file:///usr/share/jk_os/logo.png"
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true

            width: size
            height: size
        }

        PlasmaComponents3.BusyIndicator {
            id: busyIndicator
            // in the middle of the space below the logo
            y: parent.height - (parent.height - logo.y - logo.height) / 2 - height / 2
            anchors.horizontalCenter: parent.horizontalCenter
            implicitWidth: Kirigami.Units.gridUnit * 2
            implicitHeight: Kirigami.Units.gridUnit * 2
            running: true
        }

        Text {
            anchors {
                bottom: parent.bottom
                right: parent.right
                margins: Kirigami.Units.gridUnit
            }
            color: "#4d5358"
            text: "jk_os · J.K. Robotics Pvt. Ltd."
            textFormat: Text.PlainText
            Accessible.name: text
            Accessible.role: Accessible.StaticText
        }
    }

    OpacityAnimator {
        id: introAnimation
        running: false
        target: content
        from: 0
        to: 1
        duration: Kirigami.Units.veryLongDuration * 2
        easing.type: Easing.InOutQuad
    }
}
