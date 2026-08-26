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
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout, this, &X11Input::pollEvents);
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

void X11Input::setTriggerModifier(unsigned int modifier)
{
    if (m_triggerModifier == modifier) return;
    m_triggerModifier = modifier;
}

void X11Input::setTriggerButton(int button)
{
    if (m_triggerButton == button) return;
    m_triggerButton = button;
}

void X11Input::setTriggerKey(unsigned int keysym)
{
    if (m_triggerKeysym == keysym) return;
    m_triggerKeysym = keysym;
    qCInfo(lcX11Input) << "Trigger key updated:" << keysym;
}

void X11Input::startListening()
{
    if (m_listening) return;
    if (!m_display || m_rootWindow == 0) return;

    m_pollTimer->start(8);

    qCInfo(lcX11Input) << "Started listening for global input";
    m_listening = true;
}

void X11Input::stopListening()
{
    if (!m_listening) return;

    m_pollTimer->stop();

    m_superPressed = false;
    m_rmbPressed = false;
    m_keyWasDown = false;
    m_listening = false;

    qCInfo(lcX11Input) << "Stopped listening";
}

void X11Input::pollEvents()
{
    if (!m_display || !m_listening) return;

    Display* dpy = static_cast<Display*>(m_display);

    // Drain queued X events
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

    // Poll keyboard shortcut via XQueryKeymap (bypasses KWin grabs)
    pollKeyboardShortcut();
}

bool X11Input::isModifierDown(unsigned int mask)
{
    if (!m_display) return false;

    Display* dpy = static_cast<Display*>(m_display);

    char keys_return[32];
    XQueryKeymap(dpy, keys_return);

    // Check each modifier bit
    if (mask & 1) { // Shift
        KeyCode shiftL = XKeysymToKeycode(dpy, XK_Shift_L);
        KeyCode shiftR = XKeysymToKeycode(dpy, XK_Shift_R);
        if ((shiftL && (keys_return[shiftL >> 3] & (1 << (shiftL & 7)))) ||
            (shiftR && (keys_return[shiftR >> 3] & (1 << (shiftR & 7))))) {
            return true;
        }
    }
    if (mask & 4) { // Ctrl
        KeyCode ctrlL = XKeysymToKeycode(dpy, XK_Control_L);
        KeyCode ctrlR = XKeysymToKeycode(dpy, XK_Control_R);
        if ((ctrlL && (keys_return[ctrlL >> 3] & (1 << (ctrlL & 7)))) ||
            (ctrlR && (keys_return[ctrlR >> 3] & (1 << (ctrlR & 7))))) {
            return true;
        }
    }
    if (mask & 8) { // Alt
        KeyCode altL = XKeysymToKeycode(dpy, XK_Alt_L);
        KeyCode altR = XKeysymToKeycode(dpy, XK_Alt_R);
        if ((altL && (keys_return[altL >> 3] & (1 << (altL & 7)))) ||
            (altR && (keys_return[altR >> 3] & (1 << (altR & 7))))) {
            return true;
        }
    }
    if (mask & 64) { // Super
        KeyCode superL = XKeysymToKeycode(dpy, XK_Super_L);
        KeyCode superR = XKeysymToKeycode(dpy, XK_Super_R);
        if ((superL && (keys_return[superL >> 3] & (1 << (superL & 7)))) ||
            (superR && (keys_return[superR >> 3] & (1 << (superR & 7))))) {
            return true;
        }
    }

    return false;
}

void X11Input::pollKeyboardShortcut()
{
    if (m_triggerKeysym == 0) return;

    Display* dpy = static_cast<Display*>(m_display);

    char keys_return[32];
    XQueryKeymap(dpy, keys_return);

    KeyCode keycode = XKeysymToKeycode(dpy, m_triggerKeysym);
    if (!keycode) return;

    bool keyDown = (keys_return[keycode >> 3] & (1 << (keycode & 7))) != 0;

    // Rising edge: key just pressed
    if (keyDown && !m_keyWasDown) {
        qCDebug(lcX11Input) << "Keyboard shortcut pressed (poll): keysym" << m_triggerKeysym;
        if (m_keyboardCallback) {
            m_keyboardCallback();
        }
    }
    // Falling edge: key just released
    else if (!keyDown && m_keyWasDown) {
        qCDebug(lcX11Input) << "Keyboard shortcut released (poll): keysym" << m_triggerKeysym;
        if (m_superRMBReleaseCallback) {
            m_superRMBReleaseCallback();
        }
    }

    m_keyWasDown = keyDown;
}

void X11Input::handleButtonPress(int button)
{
    // Handle mouse wheel
    if (button == Button4) { handleWheel(1); return; }
    if (button == Button5) { handleWheel(-1); return; }

    if (button == m_triggerButton) {
        m_rmbPressed = true;

        if (isModifierDown(m_triggerModifier)) {
            m_superPressed = true;
            qCDebug(lcX11Input) << "Modifier + button pressed";

            if (m_superRMBCallback) {
                m_superRMBCallback();
            }
        }
    }
}

void X11Input::handleButtonRelease(int button)
{
    if (button == m_triggerButton) {
        m_rmbPressed = false;

        if (m_superPressed) {
            m_superPressed = false;

            if (m_superRMBReleaseCallback) {
                m_superRMBReleaseCallback();
            }

            qCDebug(lcX11Input) << "Modifier + button released";
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
