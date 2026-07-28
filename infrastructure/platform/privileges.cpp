#include "privileges.h"

#include <QCoreApplication>
#include <QStringList>

#if defined(Q_OS_WIN)
#include <shellapi.h>
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <unistd.h>
#endif

bool Privileges::isElevated()
{
#if defined(Q_OS_WIN)
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(
            &ntAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &adminGroup
        ))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
#elif defined(Q_OS_UNIX)
    return geteuid() == 0;
#else
    return false;
#endif
}

bool Privileges::relaunchElevated()
{
    const QString program = QCoreApplication::applicationFilePath();
    QStringList arguments = QCoreApplication::arguments();
    if (!arguments.isEmpty())
    {
        arguments.removeFirst();
    }
    if (!arguments.contains("--connect"))
    {
        arguments << "--connect";
    }

#if defined(Q_OS_WIN)
    QStringList quotedArguments;
    for (const QString &argument : arguments)
    {
        quotedArguments << (argument.contains(' ') ? "\"" + argument + "\"" : argument);
    }
    const QString joinedArguments = quotedArguments.join(' ');
    const HINSTANCE result = ShellExecuteW(
        nullptr,
        L"runas",
        reinterpret_cast<LPCWSTR>(program.utf16()),
        reinterpret_cast<LPCWSTR>(joinedArguments.utf16()),
        nullptr,
        SW_SHOWNORMAL
    );
    return reinterpret_cast<INT_PTR>(result) > 32;
#else
    Q_UNUSED(program)
    Q_UNUSED(arguments)
    return false;
#endif
}
