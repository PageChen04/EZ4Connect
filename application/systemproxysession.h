#ifndef SYSTEMPROXYSESSION_H
#define SYSTEMPROXYSESSION_H

#include <memory>

#include <QFutureWatcher>
#include <QObject>

#include "systemproxybackend.h"

class SystemProxySession : public QObject
{
Q_OBJECT

public:
    explicit SystemProxySession(QObject *parent = nullptr);
    explicit SystemProxySession(std::unique_ptr<SystemProxyBackend> backend, QObject *parent = nullptr);
    ~SystemProxySession() override;

    bool checkConflict(const SystemProxyConfig &config);
    bool enable(const SystemProxyConfig &config);
    bool disable();
    bool isEnabled() const;
    bool isBusy() const;
    void clearBeforeShutdown();

signals:
    void enabledChanged(bool enabled);
    void busyChanged(bool busy);
    void conflictCheckFinished(bool conflict);
    void operationFinished(bool enabled);

private:
    enum class Operation
    {
        None,
        CheckConflict,
        Enable,
        Disable
    };

    struct OperationResult
    {
        Operation operation;
        bool conflict = false;
    };

    bool startOperation(Operation operation, const SystemProxyConfig &config = {});
    void handleOperationFinished();

    std::unique_ptr<SystemProxyBackend> backend;
    QFutureWatcher<OperationResult> operationWatcher;
    Operation currentOperation = Operation::None;
    bool enabled = false;
};

#endif // SYSTEMPROXYSESSION_H
