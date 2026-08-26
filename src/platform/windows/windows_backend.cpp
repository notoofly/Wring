#include "windows_backend.hpp"
#include <QLoggingCategory>
#include <QImage>
#include <QProcess>
#include <shellapi.h>
#include <psapi.h>

Q_LOGGING_CATEGORY(lcWin, "wring.platform.windows")

WindowsBackend::WindowsBackend(QObject* parent)
    : DesktopBackend(parent)
{
}

WindowsBackend::~WindowsBackend()
{
    shutdown();
}

bool WindowsBackend::initialize()
{
    qCInfo(lcWin) << "Windows backend initialized";
    return true;
}

void WindowsBackend::shutdown()
{
}

QList<WindowInfo> WindowsBackend::listWindows()
{
    QList<WindowInfo> result;

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&result));

    qCDebug(lcWin) << "Found" << result.size() << "windows";
    return result;
}

BOOL CALLBACK WindowsBackend::enumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto* result = reinterpret_cast<QList<WindowInfo>*>(lParam);

    if (!IsWindowVisible(hwnd)) return TRUE;

    QString title = getWindowTitle(hwnd);
    if (title.isEmpty()) return TRUE;

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (!(style & WS_OVERLAPPEDWINDOW)) return TRUE;

    if (style & WS_EX_TOOLWINDOW) return TRUE;

    WindowInfo info;
    info.id = reinterpret_cast<PlatformWindowId>(hwnd);
    info.title = title;
    info.applicationName = getProcessName(hwnd);
    info.icon = getWindowIcon(hwnd);

    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
        info.geometry = QRect(rect.left, rect.top,
                             rect.right - rect.left,
                             rect.bottom - rect.top);
    }

    info.isActive = (GetForegroundWindow() == hwnd);

    result->append(info);
    return TRUE;
}

bool WindowsBackend::activateWindow(PlatformWindowId windowId)
{
    HWND hwnd = reinterpret_cast<HWND>(windowId);
    if (!hwnd || !IsWindow(hwnd)) return false;

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    SetForegroundWindow(hwnd);
    BringWindowToTop(hwnd);

    qCDebug(lcWin) << "Activated window:" << windowId;
    return true;
}

bool WindowsBackend::minimizeWindow(PlatformWindowId windowId)
{
    HWND hwnd = reinterpret_cast<HWND>(windowId);
    if (!hwnd || !IsWindow(hwnd)) return false;

    ShowWindow(hwnd, SW_MINIMIZE);
    return true;
}

bool WindowsBackend::closeWindow(PlatformWindowId windowId)
{
    HWND hwnd = reinterpret_cast<HWND>(windowId);
    if (!hwnd || !IsWindow(hwnd)) return false;

    PostMessage(hwnd, WM_CLOSE, 0, 0);
    return true;
}

QPoint WindowsBackend::cursorPosition()
{
    POINT pt;
    if (GetCursorPos(&pt)) {
        return QPoint(pt.x, pt.y);
    }
    return QPoint(0, 0);
}

QRect WindowsBackend::screenGeometry(const QPoint& point)
{
    POINT pt;
    pt.x = point.x();
    pt.y = point.y();

    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    if (GetMonitorInfo(mon, &mi)) {
        return QRect(mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top);
    }

    return QRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

QList<QRect> WindowsBackend::allScreenGeometries()
{
    QList<QRect> result;

    struct EnumData {
        QList<QRect>* screens;
    };

    EnumData data;
    data.screens = &result;

    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR hMon, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto* screens = reinterpret_cast<QList<QRect>*>(lParam);
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            if (GetMonitorInfo(hMon, &mi)) {
                screens->append(QRect(mi.rcMonitor.left, mi.rcMonitor.top,
                                     mi.rcMonitor.right - mi.rcMonitor.left,
                                     mi.rcMonitor.bottom - mi.rcMonitor.top));
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&data));

    return result;
}

QList<WorkspaceInfo> WindowsBackend::listWorkspaces()
{
    QList<WorkspaceInfo> result;

    result.append({0, "Desktop 1", true, 0});

    qCDebug(lcWin) << "Windows workspace support: limited (MVP)";
    return result;
}

bool WindowsBackend::switchWorkspace(int index)
{
    Q_UNUSED(index)
    qCDebug(lcWin) << "Windows workspace switching: not implemented in MVP";
    return false;
}

QList<ApplicationInfo> WindowsBackend::popularApplications()
{
    QList<ApplicationInfo> result;

    struct DefaultApp {
        const char* name;
        const char* displayName;
        const char* executable;
    };

    static const DefaultApp defaults[] = {
        {"firefox", "Firefox", "firefox.exe"},
        {"code", "VS Code", "code.exe"},
        {"terminal", "Terminal", "cmd.exe"},
        {"explorer", "Explorer", "explorer.exe"},
    };

    static const int defaultCount = sizeof(defaults) / sizeof(defaults[0]);

    for (int i = 0; i < defaultCount; ++i) {
        ApplicationInfo app;
        app.name = defaults[i].name;
        app.displayName = defaults[i].displayName;
        app.executable = defaults[i].executable;
        app.popularity = defaultCount - i;
        result.append(app);
    }

    return result;
}

bool WindowsBackend::launchApplication(const QString& executable)
{
    if (executable.isEmpty()) return false;

    SHELLEXECUTEINFO sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "open";
    sei.lpFile = executable.toStdWString().c_str();
    sei.nShow = SW_SHOWNORMAL;

    BOOL ok = ShellExecuteEx(&sei);
    if (ok) {
        qCDebug(lcWin) << "Launched application:" << executable;
    } else {
        qCWarning(lcWin) << "Failed to launch:" << executable;
    }

    return ok;
}

QString WindowsBackend::getWindowTitle(HWND hwnd)
{
    wchar_t title[512];
    int len = GetWindowTextW(hwnd, title, 512);
    if (len > 0) {
        return QString::fromWCharArray(title, len);
    }
    return {};
}

QString WindowsBackend::getProcessName(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return {};

    wchar_t name[MAX_PATH];
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameW(hProcess, 0, name, &size)) {
        CloseHandle(hProcess);
        QString path = QString::fromWCharArray(name);
        return path.section('\\', -1).section('.', 0, 0);
    }

    CloseHandle(hProcess);
    return {};
}

QImage WindowsBackend::getWindowIcon(HWND hwnd)
{
    HICON hIcon = reinterpret_cast<HICON>(
        GetClassLongPtr(hwnd, GCLP_HICON));

    if (!hIcon) {
        hIcon = reinterpret_cast<HICON>(
            SendMessage(hwnd, WM_GETICON, ICON_BIG, 0));
    }

    if (!hIcon) return {};

    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) return {};

    BITMAP bmp;
    if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp) == 0) {
        return {};
    }

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    if (width <= 0 || height <= 0) return {};

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(0);

    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, iconInfo.hbmColor);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            COLORREF pixel = GetPixel(hdc, x, y);
            image.setPixel(x, y, qRgb(GetRValue(pixel),
                                      GetGValue(pixel),
                                      GetBValue(pixel)));
        }
    }

    SelectObject(hdc, oldBmp);
    DeleteDC(hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return image;
}

bool WindowsBackend::isWindowVisible(HWND hwnd)
{
    return ::IsWindowVisible(hwnd);
}
