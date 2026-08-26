#include "x11_backend.hpp"

// Qt headers first (QTextStream::Status must be defined before X11's Status)
#include <QLoggingCategory>
#include <QImage>
#include <QFile>
#include <QProcess>
#include <cstring>

// X11 headers after Qt — Xlib.h defines Status which conflicts with Qt
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>

Q_LOGGING_CATEGORY(lcX11, "wring.platform.x11")

X11Backend::X11Backend(QObject* parent)
    : DesktopBackend(parent)
{
}

X11Backend::~X11Backend()
{
    shutdown();
}

bool X11Backend::initialize()
{
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        qCCritical(lcX11) << "Failed to open X11 display";
        return false;
    }

    m_screen = DefaultScreen(m_display);
    m_rootWindow = RootWindow(m_display, m_screen);

    int xcbScreen;
    m_xcbConnection = xcb_connect(nullptr, &xcbScreen);
    if (xcb_connection_has_error(m_xcbConnection)) {
        qCWarning(lcX11) << "Failed to connect to XCB (non-fatal)";
        m_xcbConnection = nullptr;
    }

    qCInfo(lcX11) << "X11 backend initialized, screen" << m_screen;
    return true;
}

void X11Backend::shutdown()
{
    if (m_xcbConnection) {
        xcb_disconnect(m_xcbConnection);
        m_xcbConnection = nullptr;
    }

    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}

QList<WindowInfo> X11Backend::listWindows()
{
    QList<WindowInfo> result;

    if (!m_display) return result;

    QList<unsigned long> children = getChildWindows(m_rootWindow);

    for (unsigned long w : children) {
        if (!isWindowVisible(w)) continue;

        QString title = getWindowTitle(w);
        if (title.isEmpty()) continue;

        WindowInfo info;
        info.id = static_cast<PlatformWindowId>(w);
        info.title = title;
        info.applicationName = getApplicationName(w);
        info.className = getWindowClass(w);
        info.icon = getWindowIcon(w);

        XWindowAttributes attrs;
        if (XGetWindowAttributes(m_display, w, &attrs)) {
            info.geometry = QRect(attrs.x, attrs.y, attrs.width, attrs.height);
            info.isMinimized = (attrs.map_state == IsUnmapped);
        }

        unsigned long activeWindow = 0;
        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        unsigned char* data = nullptr;

        Atom netActiveWindow = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", True);
        if (netActiveWindow != None &&
            XGetWindowProperty(m_display, m_rootWindow, netActiveWindow,
                             0, 1, False, XA_WINDOW, &actualType, &actualFormat,
                             &nItems, &bytesAfter, &data) == Success &&
            data && nItems > 0) {
            activeWindow = *reinterpret_cast<unsigned long*>(data);
            XFree(data);
        }

        info.isActive = (w == activeWindow);
        result.append(info);
    }

    qCDebug(lcX11) << "Found" << result.size() << "windows";
    return result;
}

bool X11Backend::activateWindow(PlatformWindowId windowId)
{
    if (!m_display) return false;

    unsigned long w = static_cast<unsigned long>(windowId);

    Atom netActiveWindow = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", False);
    Atom netWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
    Atom netWmStateHidden = XInternAtom(m_display, "_NET_WM_STATE_HIDDEN", False);

    XEvent event;
    std::memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = m_display;
    event.xclient.window = w;
    event.xclient.message_type = netWmState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
    event.xclient.data.l[1] = netWmStateHidden;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 1;

    XSendEvent(m_display, m_rootWindow, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);

    std::memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = m_display;
    event.xclient.window = w;
    event.xclient.message_type = netActiveWindow;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;
    event.xclient.data.l[1] = CurrentTime;
    event.xclient.data.l[2] = 0;

    XSendEvent(m_display, m_rootWindow, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);

    XRaiseWindow(m_display, w);
    XMapWindow(m_display, w);
    XFlush(m_display);

    qCDebug(lcX11) << "Activated window:" << windowId;
    return true;
}

bool X11Backend::minimizeWindow(PlatformWindowId windowId)
{
    if (!m_display) return false;

    unsigned long w = static_cast<unsigned long>(windowId);

    Atom netWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
    Atom netWmStateHidden = XInternAtom(m_display, "_NET_WM_STATE_HIDDEN", False);

    XEvent event;
    std::memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.display = m_display;
    event.xclient.window = w;
    event.xclient.message_type = netWmState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    event.xclient.data.l[1] = netWmStateHidden;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 1;

    XSendEvent(m_display, m_rootWindow, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(m_display);

    return true;
}

bool X11Backend::closeWindow(PlatformWindowId windowId)
{
    if (!m_display) return false;

    unsigned long w = static_cast<unsigned long>(windowId);

    Atom netClose = XInternAtom(m_display, "_NET_CLOSE_WINDOW", False);

    XEvent event;
    std::memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.display = m_display;
    event.xclient.window = w;
    event.xclient.message_type = netClose;
    event.xclient.format = 32;
    event.xclient.data.l[0] = CurrentTime;
    event.xclient.data.l[1] = 2;

    XSendEvent(m_display, m_rootWindow, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(m_display);

    return true;
}

QPoint X11Backend::cursorPosition()
{
    if (!m_display) return QPoint(0, 0);

    unsigned long root_ret, child_ret;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    if (XQueryPointer(m_display, m_rootWindow, &root_ret, &child_ret,
                      &root_x, &root_y, &win_x, &win_y, &mask)) {
        return QPoint(root_x, root_y);
    }

    return QPoint(0, 0);
}

QRect X11Backend::screenGeometry(const QPoint& point)
{
    QList<QRect> screens = allScreenGeometries();

    for (const QRect& screen : screens) {
        if (screen.contains(point)) {
            return screen;
        }
    }

    if (!screens.isEmpty()) {
        return screens.first();
    }

    if (m_display) {
        return QRect(0, 0, DisplayWidth(m_display, m_screen),
                     DisplayHeight(m_display, m_screen));
    }

    return QRect(0, 0, 1920, 1080);
}

QList<QRect> X11Backend::allScreenGeometries()
{
    QList<QRect> result;

    if (!m_display) return result;

    Atom netWorkarea = XInternAtom(m_display, "_NET_WORKAREA", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, m_rootWindow, netWorkarea,
                          0, 4, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems >= 4) {
        long* vals = reinterpret_cast<long*>(data);
        result.append(QRect(vals[0], vals[1], vals[2], vals[3]));
        XFree(data);
    }

    if (result.isEmpty()) {
        result.append(QRect(0, 0, DisplayWidth(m_display, m_screen),
                           DisplayHeight(m_display, m_screen)));
    }

    return result;
}

QList<WorkspaceInfo> X11Backend::listWorkspaces()
{
    QList<WorkspaceInfo> result;

    if (!m_display) return result;

    Atom netDesktops = XInternAtom(m_display, "_NET_NUMBER_OF_DESKTOPS", False);
    Atom netCurrentDesktop = XInternAtom(m_display, "_NET_CURRENT_DESKTOP", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    int numDesktops = 1;
    int currentDesktop = 0;

    if (XGetWindowProperty(m_display, m_rootWindow, netDesktops,
                          0, 1, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        numDesktops = static_cast<int>(*reinterpret_cast<long*>(data));
        XFree(data);
        data = nullptr;
    }

    if (XGetWindowProperty(m_display, m_rootWindow, netCurrentDesktop,
                          0, 1, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        currentDesktop = static_cast<int>(*reinterpret_cast<long*>(data));
        XFree(data);
        data = nullptr;
    }

    Atom netDesktopNames = XInternAtom(m_display, "_NET_DESKTOP_NAMES", False);
    QStringList names;

    if (XGetWindowProperty(m_display, m_rootWindow, netDesktopNames,
                          0, 1024, False, netDesktopNames, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        char* str = reinterpret_cast<char*>(data);
        unsigned long remaining = nItems;
        while (remaining > 0) {
            QString name = QString::fromUtf8(str);
            names.append(name);
            int len = static_cast<int>(std::strlen(str)) + 1;
            str += len;
            remaining -= len;
        }
        XFree(data);
    }

    for (int i = 0; i < numDesktops; ++i) {
        WorkspaceInfo info;
        info.index = i;
        info.name = (i < names.size()) ? names[i] : QString("Desktop %1").arg(i + 1);
        info.isActive = (i == currentDesktop);
        result.append(info);
    }

    qCDebug(lcX11) << "Found" << result.size() << "workspaces";
    return result;
}

bool X11Backend::switchWorkspace(int index)
{
    if (!m_display) return false;

    Atom netCurrentDesktop = XInternAtom(m_display, "_NET_CURRENT_DESKTOP", False);

    XEvent event;
    std::memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.display = m_display;
    event.xclient.window = m_rootWindow;
    event.xclient.message_type = netCurrentDesktop;
    event.xclient.format = 32;
    event.xclient.data.l[0] = index;
    event.xclient.data.l[1] = CurrentTime;

    XSendEvent(m_display, m_rootWindow, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(m_display);

    qCDebug(lcX11) << "Switched to workspace" << index;
    return true;
}

QList<ApplicationInfo> X11Backend::popularApplications()
{
    QList<ApplicationInfo> result;

    struct DefaultApp {
        const char* name;
        const char* displayName;
        const char* executable;
    };

    static const DefaultApp defaults[] = {
        {"firefox", "Firefox", "firefox"},
        {"code", "VS Code", "code"},
        {"terminal", "Terminal", ""},
        {"dolphin", "Dolphin", "dolphin"},
        {"thunar", "Thunar", "thunar"},
        {"nautilus", "Files", "nautilus"},
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

bool X11Backend::launchApplication(const QString& executable)
{
    if (executable.isEmpty()) return false;

    QProcess::startDetached(executable);

    qCDebug(lcX11) << "Launched application:" << executable;
    return true;
}

QList<unsigned long> X11Backend::getChildWindows(unsigned long parent)
{
    QList<unsigned long> result;

    if (!m_display) return result;

    Window root_ret, parent_ret;
    Window* children = nullptr;
    unsigned int nChildren = 0;

    if (XQueryTree(m_display, parent, &root_ret, &parent_ret, &children, &nChildren)) {
        if (children) {
            for (unsigned int i = 0; i < nChildren; ++i) {
                result.append(children[i]);
            }
            XFree(children);
        }
    }

    return result;
}

bool X11Backend::isWindowVisible(unsigned long window)
{
    if (!m_display) return false;

    XWindowAttributes attrs;
    if (!XGetWindowAttributes(m_display, window, &attrs)) {
        return false;
    }

    if (attrs.map_state != IsViewable) return false;
    if (attrs.override_redirect) return false;

    Atom netWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, window, netWmState,
                          0, 1024, False, XA_ATOM, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success) {
        if (data && nItems > 0) {
            Atom* states = reinterpret_cast<Atom*>(data);
            Atom skipTaskbar = XInternAtom(m_display, "_NET_WM_STATE_SKIP_TASKBAR", False);
            Atom skipPager = XInternAtom(m_display, "_NET_WM_STATE_SKIP_PAGER", False);

            bool skip = false;
            for (unsigned long i = 0; i < nItems; ++i) {
                if (states[i] == skipTaskbar || states[i] == skipPager) {
                    skip = true;
                    break;
                }
            }
            XFree(data);
            if (skip) return false;
        } else if (data) {
            XFree(data);
        }
    }

    return true;
}

QString X11Backend::getWindowTitle(unsigned long window)
{
    if (!m_display) return {};

    Atom netWmName = XInternAtom(m_display, "_NET_WM_NAME", False);
    Atom utf8String = XInternAtom(m_display, "UTF8_STRING", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, window, netWmName,
                          0, 1024, False, utf8String, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        QString title = QString::fromUtf8(reinterpret_cast<char*>(data), nItems);
        XFree(data);
        return title;
    }

    Atom wmName = XInternAtom(m_display, "WM_NAME", False);
    if (XGetWindowProperty(m_display, window, wmName,
                          0, 1024, False, AnyPropertyType, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        QString title = QString::fromUtf8(reinterpret_cast<char*>(data), nItems);
        XFree(data);
        return title;
    }

    return {};
}

QString X11Backend::getApplicationName(unsigned long window)
{
    if (!m_display) return {};

    Atom netWmPid = XInternAtom(m_display, "_NET_WM_PID", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, window, netWmPid,
                          0, 1, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        pid_t pid = static_cast<pid_t>(*reinterpret_cast<long*>(data));
        XFree(data);

        QString commPath = QString("/proc/%1/comm").arg(pid);
        QFile commFile(commPath);
        if (commFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString name = QString::fromUtf8(commFile.readLine()).trimmed();
            commFile.close();
            if (!name.isEmpty()) return name;
        }
    }

    QString className = getWindowClass(window);
    if (!className.isEmpty()) {
        return className;
    }

    return {};
}

QString X11Backend::getWindowClass(unsigned long window)
{
    if (!m_display) return {};

    XClassHint classHint;
    if (XGetClassHint(m_display, window, &classHint)) {
        QString name = QString::fromUtf8(classHint.res_name);
        if (classHint.res_name) XFree(classHint.res_name);
        if (classHint.res_class) XFree(classHint.res_class);
        return name;
    }

    return {};
}

QImage X11Backend::getWindowIcon(unsigned long window)
{
    if (!m_display) return {};

    Atom netWmIcon = XInternAtom(m_display, "_NET_WM_ICON", False);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, window, netWmIcon,
                          0, 1024, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 2) {
        unsigned long* iconData = reinterpret_cast<unsigned long*>(data);
        unsigned long width = iconData[0];
        unsigned long height = iconData[1];

        if (width > 0 && height > 0 && width < 256 && height < 256) {
            QImage icon(static_cast<int>(width), static_cast<int>(height),
                       QImage::Format_ARGB32);

            unsigned long pixelCount = width * height;
            for (unsigned long i = 0; i < pixelCount && i + 2 < nItems; ++i) {
                unsigned long pixel = iconData[i + 2];
                int x = static_cast<int>(i % width);
                int y = static_cast<int>(i / width);
                icon.setPixel(x, y, static_cast<QRgb>(pixel));
            }

            XFree(data);
            return icon;
        }

        XFree(data);
    }

    return {};
}

quint64 X11Backend::getCardinalProperty(unsigned long window, unsigned long atom)
{
    if (!m_display) return 0;

    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* data = nullptr;

    if (XGetWindowProperty(m_display, window, atom,
                          0, 1, False, XA_CARDINAL, &actualType, &actualFormat,
                          &nItems, &bytesAfter, &data) == Success &&
        data && nItems > 0) {
        quint64 value = *reinterpret_cast<long*>(data);
        XFree(data);
        return value;
    }

    return 0;
}
