#pragma once

#include <QObject>
#include <functional>

class GlobalInput : public QObject {
    Q_OBJECT

public:
    explicit GlobalInput(QObject* parent = nullptr);
    ~GlobalInput() override;

    using SuperRMBCallback = std::function<void()>;
    using SuperRMBReleaseCallback = std::function<void()>;
    using CursorMoveCallback = std::function<void(int, int)>;
    using WheelCallback = std::function<void(int)>;

    bool initialize();
    void shutdown();

    void setSuperRMBCallback(SuperRMBCallback cb) { m_superRMBCallback = std::move(cb); }
    void setSuperRMBReleaseCallback(SuperRMBReleaseCallback cb) { m_superRMBReleaseCallback = std::move(cb); }
    void setCursorMoveCallback(CursorMoveCallback cb) { m_cursorMoveCallback = std::move(cb); }
    void setWheelCallback(WheelCallback cb) { m_wheelCallback = std::move(cb); }

    void setTriggerModifier(unsigned int modifier);
    void setTriggerButton(int button);
    void setTriggerKey(unsigned int keysym);

    void startListening();
    void stopListening();

    bool isListening() const { return m_listening; }

signals:
    void superRMBPressed();
    void superRMBReleased();
    void cursorMoved(int x, int y);
    void wheelScrolled(int delta);

private:
    void onSuperRMB();
    void onSuperRMBRelease();
    void onCursorMove(int x, int y);
    void onWheel(int delta);

    bool m_listening = false;
    SuperRMBCallback m_superRMBCallback;
    SuperRMBReleaseCallback m_superRMBReleaseCallback;
    CursorMoveCallback m_cursorMoveCallback;
    WheelCallback m_wheelCallback;

#ifdef Q_OS_WIN
    class Impl;
    std::unique_ptr<Impl> m_impl;
#elif defined(Q_OS_LINUX)
    class Impl;
    std::unique_ptr<Impl> m_impl;
#endif
};
