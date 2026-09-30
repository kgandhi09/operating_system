/*
    jk_os's splash screen (dark), shown while Plasma starts: the J.K. Robotics
    wallpaper (logo and name), so the desktop fades in on the same picture.
    Based on Breeze's (Marco Martin, GPL-2.0-or-later); ksplashqml advances
    `stage` as startup progresses. The light Global Theme's splash is this file
    with the light wallpaper.
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

Rectangle {
    id: root
    color: "#0b0f17"

    property int stage
    readonly property string wallpaper: "file:///usr/share/wallpapers/jk_os/contents/images_dark/3840x2160.jpg"
    readonly property color footerColor: "#a1a9b1"

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

    Image {
        anchors.fill: parent
        source: root.wallpaper
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
        smooth: true
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0

        PlasmaComponents3.BusyIndicator {
            id: busyIndicator
            // below the logo and name, which sit a little above the middle
            y: parent.height * 0.74 - height / 2
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
            color: root.footerColor
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
