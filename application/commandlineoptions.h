#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QString>
#include <QStringList>

namespace CommandLineOptions
{
QString value(const QStringList &arguments, const QString &key);
}

#endif // COMMANDLINEOPTIONS_H
