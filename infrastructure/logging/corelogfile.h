#ifndef CORELOGFILE_H
#define CORELOGFILE_H

#include <QFile>
#include <QObject>
#include <QTextStream>

class CoreLogFile : public QObject
{
Q_OBJECT

public:
    explicit CoreLogFile(const QString &filePath, QObject *parent = nullptr);
    ~CoreLogFile() override;

    QString filePath() const;
    bool isOpen() const;

    void appendOutput(const QString &output);

private:
    QFile file;
    QTextStream stream;
};

#endif // CORELOGFILE_H
