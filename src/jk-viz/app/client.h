// jk-viz: the connection to jk-vizd (/run/jk-viz.sock): one JSON snapshot
// per line, about every second.
#pragma once
#include <QJsonObject>
#include <QLocalSocket>
#include <QObject>
#include <QTimer>

class VizClient : public QObject {
    Q_OBJECT
public:
    explicit VizClient(QObject *parent = nullptr);
    ~VizClient() override;
    QString status() const { return status_; }
    bool connected() const { return sock_.state() == QLocalSocket::ConnectedState; }

signals:
    void snapshot(const QJsonObject &s);
    void statusChanged();

private:
    QLocalSocket sock_;
    QTimer retry_;
    QByteArray buf_;
    QString path_, status_;
    void connectNow();
    void setStatus(const QString &s);
    void readLines();
};
