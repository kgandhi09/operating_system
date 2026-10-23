// jk-link gui's connection to jk-linkd (lib/ipc.h): the status every
// second, and the commands the window sends (pair, accept, unpair, ...).
#pragma once

#include <QLocalSocket>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class LinkClient : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString error READ error NOTIFY connectedChanged)
    Q_PROPERTY(QVariantMap status READ status NOTIFY statusChanged)

public:
    explicit LinkClient(QObject *parent = nullptr);
    bool connected() const { return connected_; }
    QString error() const { return error_; }
    QVariantMap status() const { return status_; }

    // Sends {"cmd": cmd, ...args}; commandDone() reports the answer.
    Q_INVOKABLE void command(const QString &cmd, const QVariantMap &args = {});
    Q_INVOKABLE void refresh();

signals:
    void connectedChanged();
    void statusChanged();
    void commandDone(const QString &cmd, bool ok, const QString &error, const QVariantMap &answer);

private:
    void send(const QString &cmd, const QVariantMap &args);
    void readAnswers();
    void setConnected(bool c, const QString &err = {});

    QLocalSocket sock_;
    QTimer poll_;
    QQueue<QString> pending_;   // the command each awaited answer belongs to
    QByteArray buf_;
    QVariantMap status_;
    bool connected_ = false;
    QString error_;
};
