#pragma once

#include "platform/desktop_backend.hpp"
#include <windows.h>
#include <string>

class WindowsBackend : public DesktopBackend {
    Q_OBJECT

public:
    explicit WindowsBackend(QObject* parent = nullptr);
    ~WindowsBackend() override;

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

private:
    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
    QString getWindowTitle(HWND hwnd);
    QString getProcessName(HWND hwnd);
    QImage getWindowIcon(HWND hwnd);
    bool isWindowVisible(HWND hwnd);
};
