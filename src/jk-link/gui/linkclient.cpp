#include "linkclient.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString socketPath()
{
    QString env = qEnvironmentVariable("JK_LINK_SOCKET");
    return env.isEmpty() ? QStringLiteral("/run/jk-link/socket") : env;
}

} // namespace

LinkClient::LinkClient(QObject *parent) : QObject(parent)
{
    connect(&sock_, &QLocalSocket::connected, this, [this] {
        setConnected(true);
        refresh();
    });
    connect(&sock_, &QLocalSocket::disconnected, this, [this] {
        pending_.clear();
        buf_.clear();
        setConnected(false, tr("jk-linkd stopped"));
    });
    connect(&sock_, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError) {
        pending_.clear();
        setConnected(false, tr("jk-linkd is not running (%1)").arg(socketPath()));
    });
    connect(&sock_, &QLocalSocket::readyRead, this, &LinkClient::readAnswers);
    poll_.setInterval(1000);
    connect(&poll_, &QTimer::timeout, this, &LinkClient::refresh);
    poll_.start();
    sock_.connectToServer(socketPath());
}

void LinkClient::setConnected(bool c, const QString &err)
{
    if (c == connected_ && err == error_)
        return;
    connected_ = c;
    error_ = err;
    emit connectedChanged();
}

void LinkClient::refresh()
{
    if (sock_.state() == QLocalSocket::UnconnectedState) {
        sock_.connectToServer(socketPath());
        return;
    }
    // One status request in flight at a time.
    if (sock_.state() == QLocalSocket::ConnectedState && !pending_.contains(QStringLiteral("status")))
        send(QStringLiteral("status"), {});
}

void LinkClient::command(const QString &cmd, const QVariantMap &args)
{
    if (sock_.state() != QLocalSocket::ConnectedState) {
        emit commandDone(cmd, false, error_.isEmpty() ? tr("not connected to jk-linkd") : error_, {});
        return;
    }
    send(cmd, args);
    // The change shows at once.
    send(QStringLiteral("status"), {});
}

void LinkClient::send(const QString &cmd, const QVariantMap &args)
{
    QJsonObject o = QJsonObject::fromVariantMap(args);
    o.insert(QStringLiteral("cmd"), cmd);
    sock_.write(QJsonDocument(o).toJson(QJsonDocument::Compact) + '\n');
    pending_.enqueue(cmd);
}

void LinkClient::readAnswers()
{
    buf_ += sock_.readAll();
    int nl;
    while ((nl = buf_.indexOf('\n')) >= 0) {
        QByteArray line = buf_.left(nl);
        buf_.remove(0, nl + 1);
        QString cmd = pending_.isEmpty() ? QString() : pending_.dequeue();
        QVariantMap ans = QJsonDocument::fromJson(line).object().toVariantMap();
        bool ok = ans.value(QStringLiteral("ok")).toBool();
        if (cmd == QLatin1String("status")) {
            if (ok) {
                status_ = ans;
                emit statusChanged();
            }
        } else {
            emit commandDone(cmd, ok, ans.value(QStringLiteral("error")).toString(), ans);
        }
    }
}
