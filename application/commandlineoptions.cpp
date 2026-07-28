#include "commandlineoptions.h"

QString CommandLineOptions::value(const QStringList &arguments, const QString &key)
{
    const QString prefix = key + "=";
    for (int index = 0; index < arguments.size(); ++index)
    {
        const QString &argument = arguments.at(index);
        if (argument == key)
        {
            return index + 1 < arguments.size() ? arguments.at(index + 1) : QString();
        }
        if (argument.startsWith(prefix))
        {
            return argument.mid(prefix.size());
        }
    }
    return {};
}
