#pragma once

#include <QObject>
#include <QPoint>
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

    void setSuperRMBCallback(SuperRMBCallback cb) { m_superRMBCallback = std::move(cb); }
    void setSuperRMBReleaseCallback(SuperRMBReleaseCallback cb) { m_superRMBReleaseCallback = std::move(cb); }
    void setCursorMoveCallback(CursorMoveCallback cb) { m_cursorMoveCallback = std::move(cb); }
    void setWheelCallback(WheelCallback cb) { m_wheelCallback = std::move(cb); }

    void startListening();
    void stopListening();

    bool isListening() const { return m_listening; }

private:
    void pollEvents();
    void handleButtonPress(int button);
    void handleButtonRelease(int button);
    void handleMotion(int x, int y);
    void handleWheel(int delta);

    void* m_display = nullptr;
    unsigned long m_rootWindow = 0;
    bool m_listening = false;
    bool m_superPressed = false;
    bool m_rmbPressed = false;

    SuperRMBCallback m_superRMBCallback;
    SuperRMBReleaseCallback m_superRMBReleaseCallback;
    CursorMoveCallback m_cursorMoveCallback;
    WheelCallback m_wheelCallback;

    int m_lastX = 0;
    int m_lastY = 0;
};
