#ifndef APPLICATIONLOGFILE_H
#define APPLICATIONLOGFILE_H

#include <QFile>
#include <QObject>
#include <QTextStream>

class ApplicationLogFile : public QObject
{
Q_OBJECT

public:
    explicit ApplicationLogFile(const QString &filePath, QObject *parent = nullptr);
    ~ApplicationLogFile() override;

    QString filePath() const;
    bool isOpen() const;

    void appendEntry(const QString &entry);

private:
    QFile file;
    QTextStream stream;
};

#endif // APPLICATIONLOGFILE_H
