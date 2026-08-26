#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QPoint>
#include <QImage>
#include <memory>

#include "types.hpp"

class DesktopBackend;

class WringController : public QObject {
    Q_OBJECT

public:
    enum class WringState {
        Hidden,
        Ring1,
        Ring2
    };
    Q_ENUM(WringState)

    explicit WringController(QObject* parent = nullptr);
    ~WringController() override;

    void setBackend(std::unique_ptr<DesktopBackend> backend);

    WringState state() const { return m_state; }
    bool isVisible() const { return m_state != WringState::Hidden; }
    QVariantList windows() const { return m_windows; }
    QVariantList workspaces() const { return m_workspaces; }
    QVariantList popularApps() const { return m_popularApps; }
    int selectedIndex() const { return m_selectedIndex; }
    int ring2SelectedIndex() const { return m_ring2SelectedIndex; }
    QVariantMap ring1Center() const;
    QVariantMap ring2Center() const;
    QVariantMap cursorPos() const;
    int ring1ItemCount() const;
    QVariantMap screenGeometry() const;

    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void activateSelected();
    Q_INVOKABLE void activateRing2Item();
    Q_INVOKABLE void openRing2();
    Q_INVOKABLE void closeRing2();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void selectByAngle(double angle);
    Q_INVOKABLE void selectByIndex(int index);
    Q_INVOKABLE void ring2SelectByIndex(int index);
    Q_INVOKABLE void wheelUp();
    Q_INVOKABLE void wheelDown();
    Q_INVOKABLE void ring2WheelUp();
    Q_INVOKABLE void ring2WheelDown();
    Q_INVOKABLE void setCursorPosition(int x, int y);
    Q_INVOKABLE void updateWindowList();

signals:
    void stateChanged();
    void visibleChanged();
    void windowsChanged();
    void workspacesChanged();
    void popularAppsChanged();
    void selectedIndexChanged();
    void ring2SelectedIndexChanged();
    void ring1CenterChanged();
    void ring2CenterChanged();
    void cursorPosChanged();
    void screenGeometryChanged();
    void windowActivated();
    void workspaceSwitched();
    void applicationLaunched();
    void dismiss();

private:
    void setState(WringState newState);
    void refreshWindows();
    void refreshWorkspaces();
    void refreshPopularApps();
    void selectNearestItem();
    void updateScreenGeometry();
    int totalRing1Items() const;
    double itemAngle(int index) const;
    double angleFromCursor() const;

    std::unique_ptr<DesktopBackend> m_backend;
    WringState m_state = WringState::Hidden;
    QVariantList m_windows;
    QVariantList m_workspaces;
    QVariantList m_popularApps;
    int m_selectedIndex = -1;
    int m_ring2SelectedIndex = -1;

    int m_ringCenterX = 0;
    int m_ringCenterY = 0;
    int m_mouseX = 0;
    int m_mouseY = 0;
};
