#include "client.h"

#include <QJsonDocument>
#include <QJsonParseError>

VizClient::VizClient(QObject *parent)
    : QObject(parent)
{
    path_ = qEnvironmentVariable("JK_VIZ_SOCKET", QStringLiteral("/run/jk-viz.sock"));
    retry_.setSingleShot(true);
    retry_.setInterval(2000);
    connect(&retry_, &QTimer::timeout, this, &VizClient::connectNow);
    connect(&sock_, &QLocalSocket::connected, this, [this] { setStatus(QStringLiteral("connected")); });
    connect(&sock_, &QLocalSocket::readyRead, this, &VizClient::readLines);
    connect(&sock_, &QLocalSocket::disconnected, this, [this] {
        setStatus(QStringLiteral("jk-vizd went away: reconnecting…"));
        buf_.clear();
        retry_.start();
    });
    connect(&sock_, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError e) {
        switch (e) {
        case QLocalSocket::SocketAccessError:
            setStatus(QStringLiteral("No access to %1: jk-viz is for administrators (group wheel).").arg(path_));
            break;
        case QLocalSocket::ServerNotFoundError:
        case QLocalSocket::ConnectionRefusedError:
            setStatus(QStringLiteral("jk-vizd is not running (start it: sudo /etc/init.d/S50jk-vizd start)"));
            break;
        default:
            setStatus(sock_.errorString());
        }
        if (sock_.state() == QLocalSocket::UnconnectedState)
            retry_.start();
    });
    connectNow();
}

VizClient::~VizClient()
{
    // The socket says "disconnected" as it closes: not to us any more (the
    // handlers would use the timer, already gone).
    disconnect(&sock_, nullptr, this, nullptr);
    sock_.abort();
}

void VizClient::connectNow()
{
    setStatus(QStringLiteral("connecting to jk-vizd…"));
    sock_.abort();
    sock_.connectToServer(path_, QIODevice::ReadWrite);
}

void VizClient::setStatus(const QString &s)
{
    if (s == status_)
        return;
    status_ = s;
    emit statusChanged();
}

void VizClient::readLines()
{
    buf_ += sock_.readAll();
    // Only the newest complete snapshot matters (live view).
    qsizetype end = buf_.lastIndexOf('\n');
    if (end < 0)
        return;
    qsizetype start = end > 0 ? buf_.lastIndexOf('\n', end - 1) + 1 : 0;
    QByteArray line = buf_.mid(start, end - start);
    buf_.remove(0, end + 1);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(line, &err);
    if (doc.isObject())
        emit snapshot(doc.object());
}
