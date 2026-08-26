#include "x11_input.hpp"

// Qt headers before X11 — Xlib.h defines Status which conflicts with QTextStream
#include <QLoggingCategory>
#include <QTimer>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

Q_LOGGING_CATEGORY(lcX11Input, "wring.input.x11")

X11Input::X11Input(QObject* parent)
    : QObject(parent)
{
}

X11Input::~X11Input()
{
    shutdown();
}

bool X11Input::initialize(void* display, unsigned long rootWindow)
{
    if (!display || rootWindow == 0) {
        qCCritical(lcX11Input) << "Invalid display or root window";
        return false;
    }

    m_display = display;
    m_rootWindow = rootWindow;

    qCInfo(lcX11Input) << "X11 input initialized";
    return true;
}

void X11Input::shutdown()
{
    stopListening();
    m_display = nullptr;
    m_rootWindow = 0;
}

void X11Input::startListening()
{
    if (m_listening) return;
    if (!m_display || m_rootWindow == 0) return;

    Display* dpy = static_cast<Display*>(m_display);

    XGrabButton(dpy, Button3, AnyModifier, m_rootWindow, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    qCInfo(lcX11Input) << "Started listening for global input";
    m_listening = true;
}

void X11Input::stopListening()
{
    if (!m_listening) return;

    if (m_display && m_rootWindow) {
        Display* dpy = static_cast<Display*>(m_display);
        XUngrabButton(dpy, Button3, AnyModifier, m_rootWindow);
        XFlush(dpy);
    }

    m_superPressed = false;
    m_rmbPressed = false;
    m_listening = false;

    qCInfo(lcX11Input) << "Stopped listening";
}

void X11Input::pollEvents()
{
    if (!m_display || !m_listening) return;

    Display* dpy = static_cast<Display*>(m_display);

    while (XPending(dpy)) {
        XEvent event;
        XNextEvent(dpy, &event);

        switch (event.type) {
            case ButtonPress:
                handleButtonPress(event.xbutton.button);
                break;
            case ButtonRelease:
                handleButtonRelease(event.xbutton.button);
                break;
            case MotionNotify:
                handleMotion(event.xmotion.x_root, event.xmotion.y_root);
                break;
            default:
                break;
        }
    }
}

void X11Input::handleButtonPress(int button)
{
    Display* dpy = static_cast<Display*>(m_display);
    if (!dpy) return;

    // Check for Super key
    char keys_return[32];
    XQueryKeymap(dpy, keys_return);

    KeyCode superL = XKeysymToKeycode(dpy, XK_Super_L);
    KeyCode superR = XKeysymToKeycode(dpy, XK_Super_R);

    bool superDown = false;
    if (superL && (keys_return[superL >> 3] & (1 << (superL & 7)))) {
        superDown = true;
    }
    if (superR && (keys_return[superR >> 3] & (1 << (superR & 7)))) {
        superDown = true;
    }

    if (button == Button3) { // Right mouse button
        m_rmbPressed = true;

        if (superDown) {
            m_superPressed = true;
            qCDebug(lcX11Input) << "Super + RMB detected";

            if (m_superRMBCallback) {
                m_superRMBCallback();
            }
        }
    }
}

void X11Input::handleButtonRelease(int button)
{
    if (button == Button3) {
        m_rmbPressed = false;

        if (m_superPressed) {
            m_superPressed = false;

            if (m_superRMBReleaseCallback) {
                m_superRMBReleaseCallback();
            }

            qCDebug(lcX11Input) << "Super + RMB released";
        }
    }
}

void X11Input::handleMotion(int x, int y)
{
    m_lastX = x;
    m_lastY = y;

    if (m_cursorMoveCallback) {
        m_cursorMoveCallback(x, y);
    }
}

void X11Input::handleWheel(int delta)
{
    if (m_wheelCallback) {
        m_wheelCallback(delta);
    }
}
