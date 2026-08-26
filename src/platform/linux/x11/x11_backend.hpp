#pragma once

#include "platform/desktop_backend.hpp"

// Include Qt headers before X11 to avoid Status macro conflict
#include <QObject>
#include <QString>
#include <QPoint>
#include <QRect>
#include <QImage>
#include <QList>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>

class X11Backend : public DesktopBackend {
    Q_OBJECT

public:
    explicit X11Backend(QObject* parent = nullptr);
    ~X11Backend() override;

    bool initialize() override;
    void shutdown() override;

    QList<WindowInfo> listWindows() override;
    bool activateWindow(PlatformWindowId windowId) override;
    bool minimizeWindow(PlatformWindowId windowId) override;
    bool closeWindow(PlatformWindowId windowId) override;

    QPoint cursorPosition() override;
    QRect screenGeometry(const QPoint& point) override;
    QList<QRect> allScreenGeometries() override;

    QList<WorkspaceInfo> listWorkspaces() override;
    bool switchWorkspace(int index) override;

    QList<ApplicationInfo> popularApplications() override;
    bool launchApplication(const QString& executable) override;

    Display* display() const { return m_display; }
    Window rootWindow() const { return m_rootWindow; }

private:
    QString getWindowTitle(Window window);
    QString getApplicationName(Window window);
    QString getWindowClass(Window window);
    QImage getWindowIcon(Window window);
    QList<Window> getChildWindows(Window parent);
    bool isWindowVisible(Window window);
    quint64 getCardinalProperty(Window window, Atom atom);

    Display* m_display = nullptr;
    int m_screen = 0;
    Window m_rootWindow = 0;
    xcb_connection_t* m_xcbConnection = nullptr;
};
