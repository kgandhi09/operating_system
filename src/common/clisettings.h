#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Both settings windows use the CLIs as their only source of hardware
// capabilities and as the only writer. The GUI never writes sysfs itself.
class CliSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString program READ program WRITE setProgram NOTIFY changed)
    Q_PROPERTY(bool charge READ charge WRITE setCharge NOTIFY changed)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY changed)
    Q_PROPERTY(QStringList modes READ modes NOTIFY changed)
    Q_PROPERTY(QString mode READ mode NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)

public:
    explicit CliSettings(QObject *parent = nullptr)
        : QObject(parent), program_(qEnvironmentVariable("JK_POWER_CLI", "/usr/sbin/jk-power")) {}

    QString program() const { return program_; }
    void setProgram(const QString &program) { program_ = program; emit changed(); }
    bool charge() const { return charge_; }
    void setCharge(bool charge) {
        charge_ = charge;
        program_ = charge ? qEnvironmentVariable("JK_CHARGE_CLI", "/usr/sbin/jk-charge")
                          : qEnvironmentVariable("JK_POWER_CLI", "/usr/sbin/jk-power");
        emit changed();
    }
    QVariantList rows() const { return rows_; }
    QStringList modes() const { return modes_; }
    QString mode() const { return mode_; }
    QString error() const { return error_; }
    QString message() const { return message_; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool invoke(const QStringList &args, const QString &password = {});

signals:
    void changed();

private:
    QString program_;
    bool charge_ = false;
    QVariantList rows_;
    QStringList modes_;
    QString mode_;
    QString error_;
    QString message_;

    static bool command(const QString &program, const QStringList &args,
                        const QByteArray &input, QByteArray &output);
};
