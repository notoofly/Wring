#pragma once

#include "types.hpp"
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QList>
#include <memory>

class DesktopBackend : public QObject {
    Q_OBJECT

public:
    explicit DesktopBackend(QObject* parent = nullptr) : QObject(parent) {}
    ~DesktopBackend() override = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual QList<WindowInfo> listWindows() = 0;
    virtual bool activateWindow(PlatformWindowId windowId) = 0;
    virtual bool minimizeWindow(PlatformWindowId windowId) = 0;
    virtual bool closeWindow(PlatformWindowId windowId) = 0;

    virtual QPoint cursorPosition() = 0;
    virtual QRect screenGeometry(const QPoint& point) = 0;
    virtual QList<QRect> allScreenGeometries() = 0;

    virtual QList<WorkspaceInfo> listWorkspaces() = 0;
    virtual bool switchWorkspace(int index) = 0;

    virtual QList<ApplicationInfo> popularApplications() = 0;
    virtual bool launchApplication(const QString& executable) = 0;

signals:
    void windowListChanged();
    void workspaceChanged(int index);
};
