#ifndef APPLICATIONLOGGER_H
#define APPLICATIONLOGGER_H

#include <QLoggingCategory>
#include <QObject>
#include <QTextStream>

class ApplicationLogger : public QObject
{
Q_OBJECT

public:
    explicit ApplicationLogger(QObject *parent = nullptr);
    ~ApplicationLogger() override;

    void appendCoreOutput(const QString &output);

signals:
    void entryAdded(const QString &entry);

private:
    static void qtMessageHandler(
        QtMsgType type,
        const QMessageLogContext &context,
        const QString &message
    );

    void appendQtMessage(QtMsgType type, const QString &message);
    void appendMessage(const QString &prefix, const QString &message);
    void publishEntry(const QString &entry);
    void writeStandardOutput(const QString &output);

    QTextStream standardOutput;
    QtMessageHandler previousMessageHandler = nullptr;
};

#endif // APPLICATIONLOGGER_H
