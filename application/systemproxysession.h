#ifndef SYSTEMPROXYSESSION_H
#define SYSTEMPROXYSESSION_H

#include <memory>

#include <QObject>

#include "systemproxybackend.h"

class SystemProxySession : public QObject
{
Q_OBJECT

public:
    explicit SystemProxySession(QObject *parent = nullptr);
    explicit SystemProxySession(std::unique_ptr<SystemProxyBackend> backend, QObject *parent = nullptr);

    bool hasConflict(const SystemProxyConfig &config);
    void enable(const SystemProxyConfig &config);
    void disable();
    bool isEnabled() const;

signals:
    void enabledChanged(bool enabled);

private:
    std::unique_ptr<SystemProxyBackend> backend;
    bool enabled = false;
};

#endif // SYSTEMPROXYSESSION_H
