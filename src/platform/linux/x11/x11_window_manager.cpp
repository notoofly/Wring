#include "x11_window_manager.hpp"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcX11WM, "wring.platform.x11.wm")

X11WindowManager::X11WindowManager(QObject* parent)
    : QObject(parent)
{
}

X11WindowManager::~X11WindowManager()
{
    shutdown();
}

bool X11WindowManager::initialize(void* display, unsigned long rootWindow)
{
    Q_UNUSED(display)
    Q_UNUSED(rootWindow)
    qCInfo(lcX11WM) << "X11 window manager initialized";
    return true;
}

void X11WindowManager::shutdown()
{
    qCInfo(lcX11WM) << "X11 window manager shut down";
}
