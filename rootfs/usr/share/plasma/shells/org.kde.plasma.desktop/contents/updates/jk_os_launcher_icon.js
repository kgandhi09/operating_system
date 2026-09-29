// The application launcher shows the J.K. Robotics mark (icon "jk-os")
// instead of the KDE logo. Runs once per user, on new and existing desktops;
// a launcher icon the user picked is kept.
var launchers = ["org.kde.plasma.kickoff", "org.kde.plasma.kicker", "org.kde.plasma.kickerdash"];
var allPanels = panels();
for (var i = 0; i < allPanels.length; i++) {
    for (var l = 0; l < launchers.length; l++) {
        var widgets = allPanels[i].widgets(launchers[l]);
        for (var j = 0; j < widgets.length; j++) {
            var w = widgets[j];
            w.currentConfigGroup = ["General"];
            var icon = w.readConfig("icon", "");
            if (icon === "" || icon.indexOf("start-here-kde") === 0) {
                w.writeConfig("icon", "jk-os");
            }
        }
    }
}
