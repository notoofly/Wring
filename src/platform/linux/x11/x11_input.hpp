#pragma once

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <functional>

class X11Input : public QObject {
    Q_OBJECT

public:
    explicit X11Input(QObject* parent = nullptr);
    ~X11Input() override;

    bool initialize(void* display, unsigned long rootWindow);
    void shutdown();

    using SuperRMBCallback = std::function<void()>;
    using SuperRMBReleaseCallback = std::function<void()>;
    using CursorMoveCallback = std::function<void(int, int)>;
    using WheelCallback = std::function<void(int)>;
    using KeyboardCallback = std::function<void()>;

    void setSuperRMBCallback(SuperRMBCallback cb) { m_superRMBCallback = std::move(cb); }
    void setSuperRMBReleaseCallback(SuperRMBReleaseCallback cb) { m_superRMBReleaseCallback = std::move(cb); }
    void setCursorMoveCallback(CursorMoveCallback cb) { m_cursorMoveCallback = std::move(cb); }
    void setWheelCallback(WheelCallback cb) { m_wheelCallback = std::move(cb); }
    void setKeyboardCallback(KeyboardCallback cb) { m_keyboardCallback = std::move(cb); }

    void setTriggerModifier(unsigned int modifier);
    void setTriggerButton(int button);
    void setTriggerKey(unsigned int keysym);

    void startListening();
    void stopListening();
    void updateGrab();

    bool isListening() const { return m_listening; }

private slots:
    void pollEvents();

private:
    void handleButtonPress(int button);
    void handleButtonRelease(int button);
    void handleMotion(int x, int y);
    void handleWheel(int delta);
    void handleKeyPress(int keycode);
    void handleKeyRelease(int keycode);
    bool isModifierDown(unsigned int mask);
    void updateKeyGrab();

    void* m_display = nullptr;
    unsigned long m_rootWindow = 0;
    bool m_listening = false;
    bool m_superPressed = false;
    bool m_rmbPressed = false;

    unsigned int m_triggerModifier = 64; // Mod4Mask (Super)
    int m_triggerButton = 1; // Button1 (Left)
    unsigned int m_triggerKeysym = 0; // 0 = no keyboard shortcut

    SuperRMBCallback m_superRMBCallback;
    SuperRMBReleaseCallback m_superRMBReleaseCallback;
    CursorMoveCallback m_cursorMoveCallback;
    WheelCallback m_wheelCallback;
    KeyboardCallback m_keyboardCallback;

    int m_lastX = 0;
    int m_lastY = 0;

    QTimer* m_pollTimer = nullptr;
};
