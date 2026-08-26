#include "global_input.hpp"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcInput, "wring.input")

#ifdef Q_OS_LINUX
#include "linux/x11/x11_input.hpp"
#include "linux/x11/x11_backend.hpp"
#include <memory>

class GlobalInput::Impl {
public:
    std::unique_ptr<X11Input> input;
};

GlobalInput::GlobalInput(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
    m_impl->input = std::make_unique<X11Input>(this);

    m_impl->input->setSuperRMBCallback([this]() { onSuperRMB(); });
    m_impl->input->setSuperRMBReleaseCallback([this]() { onSuperRMBRelease(); });
    m_impl->input->setCursorMoveCallback([this](int x, int y) { onCursorMove(x, y); });
    m_impl->input->setWheelCallback([this](int d) { onWheel(d); });
    m_impl->input->setKeyboardCallback([this]() { onSuperRMB(); });
}

GlobalInput::~GlobalInput() = default;

bool GlobalInput::initialize()
{
    qCInfo(lcInput) << "Initializing global input (X11)";
    return true;
}

void GlobalInput::shutdown()
{
    if (m_impl && m_impl->input) {
        m_impl->input->shutdown();
    }
}

void GlobalInput::startListening()
{
    if (!m_impl || !m_impl->input) return;

    qCInfo(lcInput) << "Starting global input listening";

    auto* x11Backend = qobject_cast<X11Backend*>(
        parent()->findChild<DesktopBackend*>());

    if (x11Backend) {
        m_impl->input->initialize(x11Backend->display(),
                                  static_cast<unsigned long>(x11Backend->rootWindow()));
    }

    m_impl->input->startListening();
    m_listening = true;
}

void GlobalInput::setTriggerModifier(unsigned int modifier)
{
    if (m_impl && m_impl->input) {
        m_impl->input->setTriggerModifier(modifier);
    }
}

void GlobalInput::setTriggerButton(int button)
{
    if (m_impl && m_impl->input) {
        m_impl->input->setTriggerButton(button);
    }
}

void GlobalInput::setTriggerKey(unsigned int keysym)
{
    if (m_impl && m_impl->input) {
        m_impl->input->setTriggerKey(keysym);
    }
}

void GlobalInput::stopListening()
{
    if (!m_impl || !m_impl->input) return;

    m_impl->input->stopListening();
    m_listening = false;
}

void GlobalInput::onSuperRMB()
{
    qCDebug(lcInput) << "Super + RMB pressed";
    emit superRMBPressed();
    if (m_superRMBCallback) m_superRMBCallback();
}

void GlobalInput::onSuperRMBRelease()
{
    qCDebug(lcInput) << "Super + RMB released";
    emit superRMBReleased();
    if (m_superRMBReleaseCallback) m_superRMBReleaseCallback();
}

void GlobalInput::onCursorMove(int x, int y)
{
    emit cursorMoved(x, y);
    if (m_cursorMoveCallback) m_cursorMoveCallback(x, y);
}

void GlobalInput::onWheel(int delta)
{
    emit wheelScrolled(delta);
    if (m_wheelCallback) m_wheelCallback(delta);
}

#elif defined(Q_OS_WIN)

#include <memory>
#include <windows.h>

class GlobalInput::Impl {
public:
    HHOOK mouseHook = nullptr;
    HHOOK keyboardHook = nullptr;
    bool superDown = false;
    bool rmbDown = false;
    GlobalInput* owner = nullptr;

    static LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0) {
            MSLLHOOKSTRUCT* hook = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

            switch (wParam) {
                case WM_RBUTTONDOWN:
                    if (owner->m_impl->superDown) {
                        owner->m_impl->rmbDown = true;
                        owner->onSuperRMB();
                    }
                    break;

                case WM_RBUTTONUP:
                    if (owner->m_impl->rmbDown) {
                        owner->m_impl->rmbDown = false;
                        owner->onSuperRMBRelease();
                    }
                    break;

                case WM_MOUSEMOVE:
                    if (owner->m_impl->rmbDown) {
                        owner->onCursorMove(hook->pt.x, hook->pt.y);
                    }
                    break;

                case WM_MOUSEWHEEL:
                    if (owner->m_impl->rmbDown) {
                        short delta = GET_WHEEL_DELTA_WPARAM(hook->mouseData);
                        owner->onWheel(delta > 0 ? 1 : -1);
                    }
                    break;
            }
        }
        return CallNextHookEx(owner->m_impl->mouseHook, nCode, wParam, lParam);
    }

    static LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0) {
            KBDLLHOOKSTRUCT* hook = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            if (hook->vkCode == VK_LWIN || hook->vkCode == VK_RWIN) {
                if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                    owner->m_impl->superDown = true;
                } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                    owner->m_impl->superDown = false;
                    if (owner->m_impl->rmbDown) {
                        owner->m_impl->rmbDown = false;
                        owner->onSuperRMBRelease();
                    }
                }
            }
        }
        return CallNextHookEx(owner->m_impl->keyboardHook, nCode, wParam, lParam);
    }
};

GlobalInput::GlobalInput(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
    m_impl->owner = this;
}

GlobalInput::~GlobalInput() = default;

bool GlobalInput::initialize()
{
    qCInfo(lcInput) << "Initializing global input (Windows)";
    return true;
}

void GlobalInput::shutdown()
{
    stopListening();
}

void GlobalInput::startListening()
{
    if (m_listening) return;

    m_impl->mouseHook = SetWindowsHookEx(WH_MOUSE_LL, Impl::mouseProc, GetModuleHandle(nullptr), 0);
    m_impl->keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, Impl::keyboardProc, GetModuleHandle(nullptr), 0);

    if (!m_impl->mouseHook || !m_impl->keyboardHook) {
        qCCritical(lcInput) << "Failed to install hooks";
        return;
    }

    m_listening = true;
    qCInfo(lcInput) << "Started listening (Windows hooks)";
}

void GlobalInput::stopListening()
{
    if (!m_listening) return;

    if (m_impl->mouseHook) {
        UnhookWindowsHookEx(m_impl->mouseHook);
        m_impl->mouseHook = nullptr;
    }
    if (m_impl->keyboardHook) {
        UnhookWindowsHookEx(m_impl->keyboardHook);
        m_impl->keyboardHook = nullptr;
    }

    m_listening = false;
    qCInfo(lcInput) << "Stopped listening";
}

void GlobalInput::onSuperRMB()
{
    emit superRMBPressed();
    if (m_superRMBCallback) m_superRMBCallback();
}

void GlobalInput::onSuperRMBRelease()
{
    emit superRMBReleased();
    if (m_superRMBReleaseCallback) m_superRMBReleaseCallback();
}

void GlobalInput::onCursorMove(int x, int y)
{
    emit cursorMoved(x, y);
    if (m_cursorMoveCallback) m_cursorMoveCallback(x, y);
}

void GlobalInput::onWheel(int delta)
{
    emit wheelScrolled(delta);
    if (m_wheelCallback) m_wheelCallback(delta);
}

#endif
