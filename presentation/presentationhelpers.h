#ifndef PRESENTATIONHELPERS_H
#define PRESENTATIONHELPERS_H

#include <QString>

class QWidget;

namespace PresentationHelpers
{
void retainSizeWhenHidden(QWidget *widget);
void showAboutDialog(QWidget *parent = nullptr);
bool confirmCredentials(const QString &username, const QString &password);
}

#endif // PRESENTATIONHELPERS_H
