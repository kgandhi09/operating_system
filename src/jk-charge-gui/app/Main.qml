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
    Connections {
        target: chargeControls
        function onChanged() {
            const row = chargeControls.rows.find(r => r.key === "driver")
            if (row)
                driverChoice.currentIndex = Math.max(0, Math.min(2, parseInt(row.current)))
        }
    }

    function runCharge(args) {
        chargeControls.invoke(args)
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
    readonly property int selectedDriver: {
        const row = chargeControls.rows.find(r => r.key === "driver")
        return row ? Number(row.current) : -1
    }
    readonly property bool directSelected: selectedDriver > 0
    readonly property bool fallbackActive: directSelected && bat.chargerOnline && !bat.directActive
    // Live panels follow the actual charger; without one, show the saved choice.
    readonly property bool showDirect: bat.directActive || (!bat.chargerOnline && directSelected)

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
        if (bat.directActive) return "Charging · SM5440 PPS"
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
        if (bat.directActive) return "Direct charging active"
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

    component ClusterPanel: Rectangle {
        default property alias content: sectionCol.data
        property string heading
        property color accent: win.idleColor
        color: "#202833"
        radius: 8
        border.color: win.lineColor
        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: sectionCol.implicitHeight + 18
        ColumnLayout {
            id: sectionCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 9
            spacing: 4
            Label {
                text: heading
                color: accent
                font.pixelSize: 12
                font.bold: true
                font.capitalization: Font.AllUppercase
                Layout.bottomMargin: 2
            }
        }
    }

    component Stat: RowLayout {
        property string label
        property string value
        property color valueColor: win.textColor
        Layout.fillWidth: true
        spacing: 4
        Label { text: label; color: win.dimColor; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
        Label { text: value; color: valueColor; font.pixelSize: 12; font.bold: true; elide: Text.ElideLeft }
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

            // Charging overview and every control live in one top panel.
            Card {
                heading: "Charging"
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.topMargin: 18
                Layout.bottomMargin: 0
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                // The battery, filled to its level.
                Item {
                    implicitWidth: 48
                    implicitHeight: 82
                    Rectangle {
                        x: 17; y: 0; width: 14; height: 6; radius: 2
                        color: win.lineColor
                    }
                    Rectangle {
                        id: shell
                        y: 6; width: 48; height: 76; radius: 7
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
                            font.pixelSize: 24
                        color: "#ffffff"
                    }
                }

                ColumnLayout {
                    spacing: 2
                    Label {
                        text: bat.capacity >= 0 ? bat.capacity + "%" : "—"
                        font.pixelSize: 42
                        font.bold: true
                        color: win.textColor
                    }
                    Label { text: headline; font.pixelSize: 18; color: win.stateColor; font.bold: true }
                    Label { text: subline; font.pixelSize: 13; color: win.dimColor }
                }

                Item { Layout.fillWidth: true }

                ColumnLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: signed(bat.directActive ? bat.directPower : bat.power, 2, "W")
                        font.pixelSize: 32
                        font.bold: true
                        color: win.flowColor(bat.directActive ? bat.directPower : bat.power)
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: bat.directActive ? "PPS input  ·  " + mA(bat.directCurrent)
                              : "Battery net  ·  " + signed(bat.ratePerHour, 1, "%/h")
                        font.pixelSize: 13
                        color: win.dimColor
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: bat.directActive ? num(bat.directVoltage, 2, "V")
                              : "average of 30 s: " + signed(bat.powerAvg, 2, "W")
                        font.pixelSize: 11
                        color: win.dimColor
                    }
                }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: win.lineColor
                    Layout.topMargin: 4
                    Layout.bottomMargin: 2
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 7
                    Label { text: "PATH"; color: win.dimColor; font.pixelSize: 12; font.bold: true }
                    ComboBox {
                        id: driverChoice
                        Layout.fillWidth: true
                        model: ["SM5714  ·  Switching  ·  up to 15 W",
                                "SM5440  ·  Direct PPS  ·  about 25 W",
                                "SM5440  ·  Direct PPS  ·  up to 45 W"]
                        currentIndex: 0
                        enabled: chargeControls.rows.some(r => r.key === "driver")
                    }
                    Button {
                        text: "Use driver"
                        enabled: driverChoice.enabled
                        highlighted: true
                        onClicked: win.runCharge(["driver", ["sm5714", "sm5440-auto", "sm5440-full"][driverChoice.currentIndex]])
                    }
                    Button { text: "Refresh"; onClicked: chargeControls.refresh() }
                }
                Label {
                    text: "Active now: " + (bat.directActive ? "SM5440 direct PPS"
                            : bat.chargerOnline ? "SM5714 switching" : "no charger connected")
                    color: bat.directActive ? win.inColor : win.dimColor
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: driverChoice.enabled
                }
                Rectangle {
                    visible: win.fallbackActive
                    Layout.fillWidth: true
                    implicitHeight: fallbackText.implicitHeight + 16
                    radius: 7
                    color: "#3b292b"
                    border.color: win.warnColor
                    Label {
                        id: fallbackText
                        anchors.fill: parent
                        anchors.margins: 8
                        text: "SM5440 PPS is selected, but SM5714 switching is active now. "
                              + "Showing SM5714 readings and controls until direct PPS becomes active."
                        color: win.warnColor
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 2
                    columns: win.showDirect ? 2 : scroll.availableWidth > 600 ? 4 : 2
                    columnSpacing: 6
                    rowSpacing: 6
                    Repeater {
                        model: chargeControls.rows.filter(r => r.key === "limit" ||
                            (win.showDirect ? r.key === "direct-current"
                                            : ["input", "current", "voltage"].includes(r.key)))
                        delegate: Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: 82
                            color: "#202833"
                            radius: 8
                            border.color: win.lineColor
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 7
                                spacing: 2
                                Label {
                                    text: ({input: "Input limit", current: "Battery current", voltage: "Full voltage",
                                            limit: "Battery care", "direct-current": "PPS input ceiling"})[modelData.key] || modelData.label
                                    color: win.textColor
                                    font.bold: true
                                    font.pixelSize: 12
                                }
                                Label {
                                    text: modelData.choices + " " + modelData.unit
                                          + (modelData.saved ? "  ·  saved " + modelData.saved : "")
                                    color: win.dimColor
                                    font.pixelSize: 10
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    TextField {
                                        id: chargeValue
                                        Layout.fillWidth: true
                                        text: modelData.current
                                        color: win.textColor
                                        selectByMouse: true
                                        inputMethodHints: Qt.ImhDigitsOnly
                                        onAccepted: win.runCharge([modelData.key, text])
                                    }
                                    Button { text: "Apply"; onClicked: win.runCharge([modelData.key, chargeValue.text]) }
                                }
                            }
                        }
                    }
                }
                Label {
                    visible: chargeControls.rows.length === 0
                    text: "This machine offers no charging controls."
                    color: win.dimColor
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

            Card {
                id: histCard
                heading: "Instrument cluster"
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.bottomMargin: 18
                property int range: 600

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    columnSpacing: 8
                    rowSpacing: 8

                    ClusterPanel {
                        heading: "Charging and discharging"
                        accent: win.inColor
                        Stat { label: "Battery net"; value: signed(bat.power, 2, "W"); valueColor: win.flowColor(bat.power) }
                        Stat {
                            label: "Current"
                            value: signed(known(bat.current) ? bat.current * 1000 : NaN, 0, "mA")
                            valueColor: win.flowColor(bat.power)
                        }
                        Stat {
                            label: "Gauge average"
                            visible: known(bat.currentAvg)
                            value: signed(known(bat.currentAvg) ? bat.currentAvg * 1000 : NaN, 0, "mA")
                        }
                        Stat { label: "Rate"; value: signed(bat.ratePerHour, 1, "%/h") }
                        Stat {
                            label: flow > 0 ? "Time to " + (bat.careEnd > 0 && bat.careEnd < 100 ? bat.careEnd + "%" : "full")
                                            : "Time left"
                            value: duration(bat.secondsLeft)
                        }
                        Stat { label: "Status"; value: bat.directActive ? "Charging" : bat.status }
                    }

                    ClusterPanel {
                        heading: "Battery"
                        accent: win.idleColor
                        Stat { label: "Voltage"; value: num(bat.voltage, 3, "V") }
                        Stat { label: "OCV"; visible: known(bat.voltageOcv); value: num(bat.voltageOcv, 3, "V") }
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

                    ClusterPanel {
                        heading: "Health"
                        accent: "#b9a0ed"
                        Stat { label: "Design"; value: amount(bat.designFull) }
                        Stat { label: "Full now"; value: amount(bat.full) }
                        Stat {
                            label: "Health"
                            value: known(bat.healthPercent) ? bat.healthPercent.toFixed(0) + "%" : "—"
                            valueColor: known(bat.healthPercent) && bat.healthPercent < 70 ? win.warnColor : win.textColor
                        }
                        Stat { label: "Cycles"; value: bat.cycleCount >= 0 ? bat.cycleCount : "—" }
                        Stat {
                            label: "State"
                            value: bat.health !== "" ? bat.health : bat.chargerHealth !== "" ? bat.chargerHealth : "—"
                            valueColor: (bat.health || bat.chargerHealth || "Good") === "Good" ? win.textColor : win.warnColor
                        }
                    }

                    ClusterPanel {
                        heading: win.showDirect ? "SM5440 · Direct PPS" : "SM5714 · Switching"
                        accent: win.outColor
                        Stat {
                            label: "Path"
                            value: bat.directActive ? "active" : win.showDirect ? "waiting for PPS" :
                                   bat.chargerOnline ? "active" : "disconnected"
                            valueColor: bat.directActive || (!win.showDirect && bat.chargerOnline) ? win.inColor : win.dimColor
                        }
                        Stat { label: "PPS input"; visible: win.showDirect && bat.directActive; value: num(bat.directPower, 2, "W") }
                        Stat { label: "PPS voltage"; visible: win.showDirect && bat.directActive; value: num(bat.directVoltage, 2, "V") }
                        Stat { label: "PPS current"; visible: win.showDirect && bat.directActive; value: mA(bat.directCurrent) }
                        Stat { label: "PPS ceiling"; visible: win.showDirect && bat.directActive; value: mA(bat.directInputLimit) }
                        Stat { label: "Pump temp"; visible: win.showDirect && bat.directActive; value: num(bat.directTemperature, 1, "°C") }
                        Stat { label: "Device"; visible: !win.showDirect && bat.chargerPresent; value: bat.chargerName }
                        Stat { label: "Phase"; visible: !win.showDirect && bat.chargeType !== ""; value: bat.chargeType }
                        Stat { label: "Input limit"; visible: !win.showDirect && known(bat.inputLimit); value: mA(bat.inputLimit) }
                        Stat {
                            label: "Current limit"
                            visible: !win.showDirect && known(bat.chargeCurrentSet)
                            value: mA(bat.chargeCurrentSet)
                        }
                        Stat { label: "Voltage limit"; visible: !win.showDirect && known(bat.chargeVoltageSet); value: num(bat.chargeVoltageSet, 2, "V") }
                        Stat {
                            label: "Care"
                            visible: bat.careEnd > 0
                            value: bat.careEnd < 100 ? "stop at " + bat.careEnd + "%" : "off (to 100%)"
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    Layout.topMargin: 6
                    Layout.bottomMargin: 4
                    color: win.lineColor
                }

                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true
                    Label {
                        text: "HISTORY"
                        color: win.dimColor
                        font.pixelSize: 13
                        font.bold: true
                        Layout.rightMargin: 10
                    }
                    ComboBox {
                        id: historyMetric
                        model: [bat.directActive ? "PPS input power" : "Battery net power", "Battery level"]
                        Layout.preferredWidth: 205
                    }
                    Item { Layout.fillWidth: true }
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
                    values: historyMetric.currentIndex === 0 ?
                            (bat.directActive ? bat.directPowerHistory : bat.powerHistory) : bat.capacityHistory
                    seconds: histCard.range
                    signedValues: historyMetric.currentIndex === 0 && !bat.directActive
                    minRange: historyMetric.currentIndex === 0 ? 1 : 100
                    unit: historyMetric.currentIndex === 0 ? " W" : "%"
                    Layout.preferredHeight: 185
                }
            }
        }
    }
}
