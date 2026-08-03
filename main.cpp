#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "SingleApplication"

#include "application/applicationlogger.h"
#include "application/applicationconstants.h"
#include "infrastructure/logging/applicationlogfile.h"
#include "infrastructure/storage/applicationpaths.h"
#include "presentation/main/mainwindow.h"

#ifndef PROJ_VER
#define PROJ_VER "unknown"
#endif

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv, false, SingleApplication::Mode::System);
    QApplication::setApplicationName(ApplicationConstants::ApplicationName);
    QApplication::setApplicationDisplayName(ApplicationConstants::ApplicationName);
    QApplication::setApplicationVersion(PROJ_VER);
    QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript, QLocale::China));

    ApplicationLogFile applicationLogFile(ApplicationPaths::logFile());
    ApplicationLogger applicationLogger;
    QObject::connect(
        &applicationLogger,
        &ApplicationLogger::entryAdded,
        &applicationLogFile,
        &ApplicationLogFile::appendEntry
    );

#if defined(Q_OS_WINDOWS)
    QApplication::setFont(QFont("Microsoft YaHei UI", QApplication::font().pointSize()));
#endif

#if defined(Q_OS_WINDOWS)
    QString translateModule = "qt";
#else
    QString translateModule = "qtbase";
#endif
    QTranslator qtTranslator;
    QString translationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    qDebug() << "Translations path:" << translationsPath << "module:" << translateModule;
    if (qtTranslator.load(QLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript, QLocale::China),
                          translateModule, QString("_"), translationsPath))
        app.installTranslator(&qtTranslator);
    else
        qDebug() << "Failed to load transaction file for" << translateModule;

    MainWindow mainWindow(&applicationLogger, &applicationLogFile);

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    return QApplication::exec();
}
