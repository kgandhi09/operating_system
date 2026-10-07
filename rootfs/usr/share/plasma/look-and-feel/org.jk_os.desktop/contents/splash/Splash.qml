/* Particle intro for jk-gui. The wallpaper beneath the animation is the
 * desktop's default light wallpaper, so Plasma can replace the splash
 * without changing the final picture. */

import QtQuick

Rectangle {
    id: root
    color: "#f6f8f7"

    property int stage
    property bool splashFinished: false
    readonly property string wallpaper: "file:///usr/share/wallpapers/jk_os/contents/images/3840x2160.jpg"

    Image {
        anchors.fill: parent
        source: root.wallpaper
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
    }

    AnimatedImage {
        id: particles
        anchors.fill: parent
        source: "particle-splash.webp"
        fillMode: Image.PreserveAspectCrop
        cache: false
        playing: true

        onPlayingChanged: {
            if (!playing && status === Image.Ready && !reveal.running)
                reveal.start();
        }
        onStatusChanged: {
            if (status === Image.Error)
                root.splashFinished = true;
        }
    }

    NumberAnimation {
        id: reveal
        target: particles
        property: "opacity"
        from: 1
        to: 0
        duration: 450
        easing.type: Easing.InOutQuad
        onFinished: root.splashFinished = true
    }
}
