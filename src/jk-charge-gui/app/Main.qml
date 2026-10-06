import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import JkChargeGui

ApplicationWindow {
    id: win
    width: 1000
    height: 900
    visible: true
    title: "Charge Settings"
    color: "#14181e"
    palette.window: "#14181e"
    palette.windowText: "#e6e9ee"
    palette.base: "#0e1116"
    palette.text: "#e6e9ee"
    palette.button: "#232a34"
    palette.buttonText: "#e6e9ee"
    palette.highlight: "#3d7fd6"
    palette.highlightedText: "#ffffff"

    readonly property color textColor: "#e6e9ee"
    readonly property color dimColor: "#8a93a1"
    readonly property color cardColor: "#1a1f27"
    readonly property color lineColor: "#2c3440"
    readonly property color inColor: "#5cc98a"     // into the battery
    readonly property color outColor: "#f0954a"    // out of it
    readonly property color idleColor: "#5aa7f0"
    readonly property color warnColor: "#e5737d"

    BatteryMonitor { id: bat }
    CliSettings {
        id: chargeControls
        charge: true
        Component.onCompleted: refresh()
    }

    function runCharge(args) {
        if (chargeControls.invoke(args, chargePassword.text))
            chargePassword.clear()
    }

    // ---- formatting ----
    function known(v) { return v !== undefined && v !== null && !isNaN(v) }
    function num(v, digits, unit) {
        return known(v) ? v.toFixed(digits) + (unit ? " " + unit : "") : "—"
    }
    function signed(v, digits, unit) {
        return known(v) ? (v > 0 ? "+" : "") + v.toFixed(digits) + (unit ? " " + unit : "") : "—"
    }
    function duration(s) {
        if (s < 0) return "—"
        const h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60)
        return h > 0 ? h + " h " + m + " min" : m + " min"
    }
    readonly property string unitNow: bat.energyUnits ? "Wh" : "mAh"
    function amount(v) {
        if (!known(v)) return "—"
        return bat.energyUnits ? v.toFixed(1) + " Wh" : Math.round(v * 1000) + " mAh"
    }
    function mA(v) { return known(v) ? Math.round(v * 1000) + " mA" : "—" }

    readonly property real flow: known(bat.powerAvg) ? bat.powerAvg : 0
    readonly property bool held: bat.status === "Not charging" && bat.chargerOnline
                                 && bat.careEnd > 0 && bat.careEnd < 100 && bat.capacity >= bat.careEnd - 5
    // The state follows the 30 s average; the live numbers their own sign.
    function flowColor(v) {
        return !known(v) ? dimColor : v > 0.05 ? inColor : v < -0.05 ? outColor : idleColor
    }
    readonly property color stateColor: bat.available ? flowColor(flow) : dimColor
    readonly property string headline: {
        if (!bat.available) return "No battery found"
        if (bat.status === "Full") return "Full"
        if (held) return "Held at " + bat.careEnd + "% (battery care)"
        if (bat.chargerOnline && flow < -0.05)
            return "Plugged in, but discharging"
        if (flow > 0.05) return "Charging"
        if (flow < -0.05) return "Discharging"
        return bat.status
    }
    readonly property string subline: {
        if (!bat.available) return "No battery in /sys/class/power_supply"
        if (bat.chargerOnline && flow < -0.05)
            return "The device uses more power than the charger gives"
        if (bat.secondsLeft >= 0)
            return duration(bat.secondsLeft) + (flow > 0
                ? " to " + (bat.careEnd > 0 && bat.careEnd < 100 ? bat.careEnd + "%" : "full")
                : " left")
        return bat.chargerOnline ? "Charger connected" : "On battery"
    }

    // ---- building blocks ----
    component Card: Rectangle {
        default property alias content: col.data
        property string heading
        color: win.cardColor
        radius: 10
        border.color: win.lineColor
        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: col.implicitHeight + 28
        ColumnLayout {
            id: col
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 14
            spacing: 6
            Label {
                text: heading
                color: win.dimColor
                font.pixelSize: 13
                font.bold: true
                font.capitalization: Font.AllUppercase
                Layout.bottomMargin: 4
            }
        }
    }

    component Stat: RowLayout {
        property string label
        property string value
        property color valueColor: win.textColor
        Layout.fillWidth: true
        Label { text: label; color: win.dimColor; font.pixelSize: 15; Layout.fillWidth: true }
        Label { text: value; color: valueColor; font.pixelSize: 15; font.bold: true }
    }

    // History graph: the last <seconds> of <values>, newest at the right.
    component Graph: Canvas {
        property var values: []
        property int seconds: 600
        property bool signedValues: false
        property real minRange: 1
        property string unit
        Layout.fillWidth: true
        Layout.preferredHeight: 170
        onValuesChanged: requestPaint()
        onSecondsChanged: requestPaint()
        onWidthChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const w = width, h = height, padL = 46, padB = 18, padT = 8
            const n = Math.min(values.length, seconds)
            const data = values.slice(values.length - n)
            let lo = 0, hi = minRange
            for (const v of data) { hi = Math.max(hi, v); lo = Math.min(lo, v) }
            if (signedValues) { const m = Math.max(hi, -lo, minRange); hi = m; lo = -m }
            else { lo = 0; hi = Math.max(hi, minRange) }
            const y = v => padT + (h - padB - padT) * (1 - (v - lo) / (hi - lo))
            const x = i => padL + (w - padL) * (seconds - n + i) / Math.max(1, seconds - 1)

            ctx.font = "11px sans-serif"
            ctx.fillStyle = win.dimColor
            ctx.strokeStyle = win.lineColor
            ctx.lineWidth = 1
            const ticks = signedValues ? [hi, hi / 2, 0, lo / 2, lo] : [hi, hi / 2, 0]
            for (const t of ticks) {
                ctx.beginPath(); ctx.moveTo(padL, y(t)); ctx.lineTo(w, y(t)); ctx.stroke()
                ctx.fillText((Math.abs(t) >= 10 ? t.toFixed(0) : t.toFixed(1)) + unit, 0, y(t) + 4)
            }
            const mins = seconds / 60
            ctx.fillText("−" + mins + " min", padL, h - 3)
            ctx.fillText("now", w - 24, h - 3)
            if (n < 2) return

            const zero = y(0)
            // Filled area, green above zero, orange below.
            for (const above of [true, false]) {
                ctx.beginPath()
                ctx.moveTo(x(0), zero)
                for (let i = 0; i < n; i++) {
                    const v = above ? Math.max(data[i], 0) : Math.min(data[i], 0)
                    ctx.lineTo(x(i), y(v))
                }
                ctx.lineTo(x(n - 1), zero)
                ctx.closePath()
                const c = signedValues ? (above ? win.inColor : win.outColor) : win.idleColor
                ctx.fillStyle = Qt.rgba(c.r, c.g, c.b, 0.25)
                ctx.fill()
            }
            ctx.beginPath()
            for (let i = 0; i < n; i++)
                i === 0 ? ctx.moveTo(x(i), y(data[i])) : ctx.lineTo(x(i), y(data[i]))
            ctx.strokeStyle = signedValues ? win.textColor : win.idleColor
            ctx.lineWidth = 1.5
            ctx.stroke()
        }
    }

    // ---- the page ----
    ScrollView {
        id: scroll
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 14

            // Level, state and the big numbers.
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 18
                Layout.bottomMargin: 0
                spacing: 22

                // The battery, filled to its level.
                Item {
                    implicitWidth: 64
                    implicitHeight: 118
                    Rectangle {
                        x: 22; y: 0; width: 20; height: 8; radius: 2
                        color: win.lineColor
                    }
                    Rectangle {
                        id: shell
                        y: 8; width: 64; height: 110; radius: 9
                        color: "transparent"
                        border.color: win.lineColor
                        border.width: 3
                        Rectangle {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                            anchors.margins: 6
                            height: Math.max(0, (shell.height - 12) * Math.max(bat.capacity, 0) / 100)
                            radius: 4
                            color: bat.capacity >= 0 && bat.capacity <= 10 && flow <= 0 ? win.warnColor : win.stateColor
                            Behavior on height { NumberAnimation { duration: 400 } }
                        }
                    }
                    Label {
                        anchors.centerIn: shell
                        visible: flow > 0.05
                        text: "⚡"
                        font.pixelSize: 34
                        color: "#ffffff"
                    }
                }

                ColumnLayout {
                    spacing: 2
                    Label {
                        text: bat.capacity >= 0 ? bat.capacity + "%" : "—"
                        font.pixelSize: 54
                        font.bold: true
                        color: win.textColor
                    }
                    Label { text: headline; font.pixelSize: 22; color: win.stateColor; font.bold: true }
                    Label { text: subline; font.pixelSize: 15; color: win.dimColor }
                }

                Item { Layout.fillWidth: true }

                ColumnLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: signed(bat.power, 2, "W")
                        font.pixelSize: 40
                        font.bold: true
                        color: win.flowColor(bat.power)
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: (bat.energyUnits ? "" : signed(known(bat.current) ? bat.current * 1000 : NaN, 0, "mA") + "  ·  ")
                              + signed(bat.ratePerHour, 1, "%/h")
                        font.pixelSize: 16
                        color: win.dimColor
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: "average of 30 s: " + signed(bat.powerAvg, 2, "W")
                        font.pixelSize: 13
                        color: win.dimColor
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                columns: scroll.availableWidth > 1300 ? 4 : scroll.availableWidth > 620 ? 2 : 1
                columnSpacing: 14
                rowSpacing: 14

                Card {
                    heading: "Charging and discharging"
                    Stat { label: "Power"; value: signed(bat.power, 2, "W"); valueColor: win.flowColor(bat.power) }
                    Stat {
                        label: "Current"
                        value: signed(known(bat.current) ? bat.current * 1000 : NaN, 0, "mA")
                        valueColor: win.flowColor(bat.power)
                    }
                    Stat {
                        label: "Current (gauge average)"
                        visible: known(bat.currentAvg)
                        value: signed(known(bat.currentAvg) ? bat.currentAvg * 1000 : NaN, 0, "mA")
                    }
                    Stat { label: "Rate"; value: signed(bat.ratePerHour, 1, "% per hour") }
                    Stat {
                        label: flow > 0 ? "Time to " + (bat.careEnd > 0 && bat.careEnd < 100 ? bat.careEnd + "%" : "full")
                                        : "Time left"
                        value: duration(bat.secondsLeft)
                    }
                    Stat { label: "Status (driver)"; value: bat.status }
                }

                Card {
                    heading: "Battery"
                    Stat { label: "Voltage"; value: num(bat.voltage, 3, "V") }
                    Stat { label: "Resting voltage (OCV)"; visible: known(bat.voltageOcv); value: num(bat.voltageOcv, 3, "V") }
                    Stat {
                        label: "Temperature"
                        value: num(bat.temperature, 1, "°C")
                        valueColor: known(bat.temperature) && (bat.temperature >= 45 || bat.temperature < 5)
                                    ? win.warnColor : win.textColor
                    }
                    Stat { label: "Charge now"; value: amount(known(bat.now) ? bat.now
                        : (known(bat.full) ? bat.full : bat.designFull) * Math.max(bat.capacity, 0) / 100) }
                    Stat { label: "Technology"; visible: bat.technology !== ""; value: bat.technology }
                    Stat { label: "Model"; visible: bat.model !== ""; value: bat.model }
                }

                Card {
                    heading: "Health"
                    Stat { label: "Design capacity"; value: amount(bat.designFull) }
                    Stat { label: "Full capacity now"; value: amount(bat.full) }
                    Stat {
                        label: "Health"
                        value: known(bat.healthPercent) ? bat.healthPercent.toFixed(0) + "%" : "—"
                        valueColor: known(bat.healthPercent) && bat.healthPercent < 70 ? win.warnColor : win.textColor
                    }
                    Stat { label: "Charge cycles"; value: bat.cycleCount >= 0 ? bat.cycleCount : "—" }
                    Stat {
                        label: "Battery state"
                        value: bat.health !== "" ? bat.health : bat.chargerHealth !== "" ? bat.chargerHealth : "—"
                        valueColor: (bat.health || bat.chargerHealth || "Good") === "Good" ? win.textColor : win.warnColor
                    }
                    Label {
                        visible: bat.available && !known(bat.full)
                        text: "This battery's fuel gauge doesn't report how much it has worn."
                        color: win.dimColor
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }

                Card {
                    heading: "Charger"
                    Stat {
                        label: "Charger"
                        value: !bat.chargerPresent ? "none" : bat.chargerOnline ? "connected" : "not connected"
                        valueColor: bat.chargerOnline ? win.inColor : win.textColor
                    }
                    Stat { label: "Name"; visible: bat.chargerPresent; value: bat.chargerName }
                    Stat { label: "Charge phase"; visible: bat.chargeType !== ""; value: bat.chargeType }
                    Stat { label: "Input limit"; visible: known(bat.inputLimit); value: mA(bat.inputLimit) }
                    Stat {
                        label: "Battery current"
                        visible: known(bat.chargeCurrentSet)
                        value: mA(bat.chargeCurrentSet) + (known(bat.chargeCurrentMax) ? " of " + mA(bat.chargeCurrentMax) : "")
                    }
                    Stat { label: "Full voltage"; visible: known(bat.chargeVoltageSet); value: num(bat.chargeVoltageSet, 2, "V") }
                    Stat {
                        label: "Battery care"
                        visible: bat.careEnd > 0
                        value: bat.careEnd < 100 ? "stop at " + bat.careEnd + "%" : "off (to 100%)"
                    }
                    Label {
                        text: "Charging controls"
                        color: win.textColor
                        font.bold: true
                        Layout.topMargin: 12
                    }
                    Label {
                        visible: chargeControls.rows.length === 0
                        text: "This machine offers no charging controls."
                        color: win.dimColor
                    }
                    TextField {
                        id: chargePassword
                        visible: chargeControls.rows.length > 0
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: "Administrator password for changes"
                        color: win.textColor
                        placeholderTextColor: win.dimColor
                    }
                    RowLayout {
                        visible: chargeControls.rows.length > 0
                        spacing: 6
                        Button { text: "Fast"; onClicked: win.runCharge(["fast"]) }
                        Button { text: "Normal"; onClicked: win.runCharge(["normal"]) }
                        Button { text: "Gentle"; onClicked: win.runCharge(["gentle"]) }
                        Button { text: "Refresh"; onClicked: chargeControls.refresh() }
                    }
                    Repeater {
                        model: chargeControls.rows
                        delegate: RowLayout {
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 6
                            Label {
                                text: modelData.label + " (" + modelData.choices + " " + modelData.unit + ")"
                                      + (modelData.saved ? " · saved " + modelData.saved : "")
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            TextField {
                                id: chargeValue
                                text: modelData.current
                                color: win.textColor
                                Layout.preferredWidth: 84
                                selectByMouse: true
                            }
                            Button {
                                text: "Apply"
                                onClicked: win.runCharge([modelData.key, chargeValue.text])
                            }
                        }
                    }
                    Label {
                        visible: chargeControls.error !== ""
                        text: chargeControls.error
                        color: win.warnColor
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                    Label {
                        visible: chargeControls.error === "" && chargeControls.message !== ""
                        text: chargeControls.message.split("\n")[0]
                        color: win.inColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }

            // The last minutes as graphs.
            Card {
                id: histCard
                heading: "History"
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.bottomMargin: 18
                property int range: 600

                RowLayout {
                    spacing: 6
                    Label { text: "Power, into (+) and out of (−) the battery"; color: win.textColor; Layout.fillWidth: true }
                    Repeater {
                        model: [{ t: "5 min", s: 300 }, { t: "10 min", s: 600 }, { t: "30 min", s: 1800 }, { t: "1 h", s: 3600 }]
                        Button {
                            required property var modelData
                            text: modelData.t
                            highlighted: histCard.range === modelData.s
                            onClicked: histCard.range = modelData.s
                        }
                    }
                }
                Graph {
                    values: bat.powerHistory
                    seconds: histCard.range
                    signedValues: true
                    minRange: 1
                    unit: " W"
                }
                Label { text: "Battery level"; color: win.textColor; Layout.topMargin: 6 }
                Graph {
                    values: bat.capacityHistory
                    seconds: histCard.range
                    minRange: 100
                    unit: "%"
                    Layout.preferredHeight: 120
                }
            }
        }
    }
}
