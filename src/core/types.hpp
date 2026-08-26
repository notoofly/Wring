#pragma once

#include <QString>
#include <QPoint>
#include <QRect>
#include <QImage>
#include <QList>

#ifdef Q_OS_WIN
using PlatformWindowId = unsigned long long;
#else
using PlatformWindowId = unsigned long;
#endif

struct WindowInfo {
    PlatformWindowId id = 0;
    QString title;
    QString applicationName;
    QString className;
    QImage icon;
    bool isActive = false;
    bool isMinimized = false;
    bool isMaximized = false;
    QRect geometry;
};

struct WorkspaceInfo {
    int index = 0;
    QString name;
    bool isActive = false;
    int windowCount = 0;
};

struct ApplicationInfo {
    QString name;
    QString displayName;
    QString executable;
    QString desktopFile;
    QImage icon;
    int popularity = 0;
};
