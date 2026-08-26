#pragma once

#include "platform/desktop_backend.hpp"

// Forward-declare X11 types to avoid Status macro conflict with Qt.
// All X11 system headers are included only in the .cpp file.
struct _XDisplay;
using Display = _XDisplay;
struct xcb_connection_t;

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
    unsigned long rootWindow() const { return m_rootWindow; }

private:
    QString getWindowTitle(unsigned long window);
    QString getApplicationName(unsigned long window);
    QString getWindowClass(unsigned long window);
    QImage getWindowIcon(unsigned long window);
    QList<unsigned long> getChildWindows(unsigned long parent);
    bool isWindowVisible(unsigned long window);
    quint64 getCardinalProperty(unsigned long window, unsigned long atom);

    Display* m_display = nullptr;
    int m_screen = 0;
    unsigned long m_rootWindow = 0;
    xcb_connection_t* m_xcbConnection = nullptr;
};
