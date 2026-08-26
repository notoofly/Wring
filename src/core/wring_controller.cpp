#include "wring_controller.hpp"
#include "desktop_backend.hpp"
#include <QLoggingCategory>
#include <QtMath>
#include <QGuiApplication>
#include <QScreen>

Q_LOGGING_CATEGORY(lcWring, "wring.core")

WringController::WringController(QObject* parent)
    : QObject(parent)
{
}

WringController::~WringController() = default;

void WringController::setBackend(std::unique_ptr<DesktopBackend> backend)
{
    m_backend = std::move(backend);
    if (m_backend) {
        connect(m_backend.get(), &DesktopBackend::windowListChanged,
                this, &WringController::refreshWindows);
        connect(m_backend.get(), &DesktopBackend::workspaceChanged,
                this, &WringController::refreshWorkspaces);
    }
}

QVariantMap WringController::ring1Center() const
{
    return {{"x", m_ringCenterX}, {"y", m_ringCenterY}};
}

QVariantMap WringController::ring2Center() const
{
    if (m_state != WringState::Ring2) {
        return {{"x", 0}, {"y", 0}};
    }

    int count = m_windows.size() + 1;
    if (count == 0) return {{"x", m_ringCenterX}, {"y", m_ringCenterY}};

    double startAngle = -M_PI / 2.0;
    double angleStep = (2.0 * M_PI) / count;
    double radius = 150.0;

    int ring2ButtonIndex = m_windows.size();
    double angle = startAngle + ring2ButtonIndex * angleStep;

    double x = m_ringCenterX + qCos(angle) * radius;
    double y = m_ringCenterY + qSin(angle) * radius;

    return {{"x", x}, {"y", y}};
}

QVariantMap WringController::cursorPos() const
{
    return {{"x", m_mouseX}, {"y", m_mouseY}};
}

int WringController::ring1ItemCount() const
{
    return m_windows.size() + 1;
}

QVariantMap WringController::screenGeometry() const
{
    if (!m_backend) return {{"x", 0}, {"y", 0}, {"width", 1920}, {"height", 1080}};

    QPoint pos(m_ringCenterX, m_ringCenterY);
    QRect geo = m_backend->screenGeometry(pos);
    return {{"x", geo.x()}, {"y", geo.y()}, {"width", geo.width()}, {"height", geo.height()}};
}

void WringController::show()
{
    if (m_state != WringState::Hidden) return;

    if (m_backend) {
        QPoint cursor = m_backend->cursorPosition();
        m_ringCenterX = cursor.x();
        m_ringCenterY = cursor.y();
        m_mouseX = cursor.x();
        m_mouseY = cursor.y();
        emit cursorPosChanged();
        emit ring1CenterChanged();
        updateScreenGeometry();
    }

    setState(WringState::Ring1);
    refreshWindows();
    refreshWorkspaces();
    refreshPopularApps();

    qCDebug(lcWring) << "Wring shown at" << m_ringCenterX << m_ringCenterY;
}

void WringController::hide()
{
    if (m_state == WringState::Hidden) return;

    setState(WringState::Hidden);
    m_selectedIndex = -1;
    m_ring2SelectedIndex = -1;

    qCDebug(lcWring) << "Wring hidden";
    emit dismiss();
}

void WringController::toggle()
{
    if (m_state == WringState::Hidden) {
        show();
    } else {
        hide();
    }
}

void WringController::activateSelected()
{
    if (m_state != WringState::Ring1) return;
    if (m_selectedIndex < 0) return;

    int total = totalRing1Items();
    if (m_selectedIndex >= total) return;

    if (m_selectedIndex == m_windows.size()) {
        openRing2();
        return;
    }

    QVariantMap win = m_windows[m_selectedIndex].toMap();
    PlatformWindowId id = static_cast<PlatformWindowId>(win["id"].toULongLong());

    if (m_backend) {
        bool ok = m_backend->activateWindow(id);
        if (ok) {
            qCDebug(lcWring) << "Activated window:" << win["title"].toString();
            emit windowActivated();
        } else {
            qCWarning(lcWring) << "Failed to activate window:" << win["title"].toString();
        }
    }

    hide();
}

void WringController::activateRing2Item()
{
    if (m_state != WringState::Ring2) return;
    if (m_ring2SelectedIndex < 0) return;

    int workspaceCount = m_workspaces.size();
    int appCount = m_popularApps.size();

    if (m_ring2SelectedIndex < workspaceCount) {
        QVariantMap ws = m_workspaces[m_ring2SelectedIndex].toMap();
        int index = ws["index"].toInt();

        if (m_backend) {
            bool ok = m_backend->switchWorkspace(index);
            if (ok) {
                qCDebug(lcWring) << "Switched to workspace:" << ws["name"].toString();
                emit workspaceSwitched();
            } else {
                qCWarning(lcWring) << "Failed to switch workspace";
            }
        }

        hide();
    } else if (m_ring2SelectedIndex < workspaceCount + appCount) {
        int appIndex = m_ring2SelectedIndex - workspaceCount;
        QVariantMap app = m_popularApps[appIndex].toMap();
        QString exe = app["executable"].toString();

        if (m_backend) {
            bool ok = m_backend->launchApplication(exe);
            if (ok) {
                qCDebug(lcWring) << "Launched application:" << app["name"].toString();
                emit applicationLaunched();
            } else {
                qCWarning(lcWring) << "Failed to launch:" << exe;
            }
        }

        hide();
    }
}

void WringController::openRing2()
{
    if (m_state != WringState::Ring1) return;
    setState(WringState::Ring2);
    m_ring2SelectedIndex = 0;
    emit ring2SelectedIndexChanged();
    emit ring2CenterChanged();

    qCDebug(lcWring) << "Ring 2 opened";
}

void WringController::closeRing2()
{
    if (m_state != WringState::Ring2) return;
    setState(WringState::Ring1);
    m_ring2SelectedIndex = -1;
    emit ring2SelectedIndexChanged();

    qCDebug(lcWring) << "Ring 2 closed, back to Ring 1";
}

void WringController::goBack()
{
    if (m_state == WringState::Ring2) {
        closeRing2();
    } else {
        hide();
    }
}

void WringController::selectByAngle(double angle)
{
    if (m_state != WringState::Ring1) return;

    int total = totalRing1Items();
    if (total == 0) return;

    double startAngle = -M_PI / 2.0;
    double angleStep = (2.0 * M_PI) / total;

    double normalizedAngle = angle - startAngle;
    if (normalizedAngle < 0) normalizedAngle += 2.0 * M_PI;

    int index = qRound(normalizedAngle / angleStep) % total;

    if (index != m_selectedIndex) {
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }
}

void WringController::selectByIndex(int index)
{
    if (m_state != WringState::Ring1) return;

    int total = totalRing1Items();
    if (index < 0 || index >= total) return;

    if (index != m_selectedIndex) {
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }
}

void WringController::ring2SelectByIndex(int index)
{
    if (m_state != WringState::Ring2) return;

    int total = m_workspaces.size() + m_popularApps.size();
    if (index < 0 || index >= total) return;

    if (index != m_ring2SelectedIndex) {
        m_ring2SelectedIndex = index;
        emit ring2SelectedIndexChanged();
    }
}

void WringController::wheelUp()
{
    if (m_state == WringState::Ring1) {
        int total = totalRing1Items();
        if (total == 0) return;

        m_selectedIndex--;
        if (m_selectedIndex < 0) m_selectedIndex = total - 1;
        emit selectedIndexChanged();
    } else if (m_state == WringState::Ring2) {
        ring2WheelUp();
    }
}

void WringController::wheelDown()
{
    if (m_state == WringState::Ring1) {
        int total = totalRing1Items();
        if (total == 0) return;

        m_selectedIndex++;
        if (m_selectedIndex >= total) m_selectedIndex = 0;
        emit selectedIndexChanged();
    } else if (m_state == WringState::Ring2) {
        ring2WheelDown();
    }
}

void WringController::ring2WheelUp()
{
    int total = m_workspaces.size() + m_popularApps.size();
    if (total == 0) return;

    m_ring2SelectedIndex--;
    if (m_ring2SelectedIndex < 0) m_ring2SelectedIndex = total - 1;
    emit ring2SelectedIndexChanged();
}

void WringController::ring2WheelDown()
{
    int total = m_workspaces.size() + m_popularApps.size();
    if (total == 0) return;

    m_ring2SelectedIndex++;
    if (m_ring2SelectedIndex >= total) m_ring2SelectedIndex = 0;
    emit ring2SelectedIndexChanged();
}

void WringController::setCursorPosition(int x, int y)
{
    if (m_mouseX == x && m_mouseY == y) return;

    m_mouseX = x;
    m_mouseY = y;
    emit cursorPosChanged();

    if (m_state == WringState::Ring1) {
        selectNearestItem();
    }
}

void WringController::updateWindowList()
{
    refreshWindows();
}

void WringController::setState(WringState newState)
{
    if (m_state == newState) return;
    m_state = newState;
    emit stateChanged();
    emit visibleChanged();
}

void WringController::refreshWindows()
{
    if (!m_backend) return;

    QList<WindowInfo> wins = m_backend->listWindows();
    QVariantList list;

    for (const auto& w : wins) {
        QVariantMap map;
        map["id"] = QVariant::fromValue(static_cast<quint64>(w.id));
        map["title"] = w.title;
        map["applicationName"] = w.applicationName;
        map["className"] = w.className;
        map["isActive"] = w.isActive;
        map["isMinimized"] = w.isMinimized;
        map["isMaximized"] = w.isMaximized;
        map["x"] = w.geometry.x();
        map["y"] = w.geometry.y();
        map["width"] = w.geometry.width();
        map["height"] = w.geometry.height();

        if (!w.icon.isNull()) {
            map["icon"] = QVariant::fromValue(w.icon);
        }

        list.append(map);
    }

    m_windows = list;
    emit windowsChanged();

    if (m_state == WringState::Ring1 && m_selectedIndex < 0) {
        selectNearestItem();
    }
}

void WringController::refreshWorkspaces()
{
    if (!m_backend) return;

    QList<WorkspaceInfo> wss = m_backend->listWorkspaces();
    QVariantList list;

    for (const auto& ws : wss) {
        QVariantMap map;
        map["index"] = ws.index;
        map["name"] = ws.name;
        map["isActive"] = ws.isActive;
        map["windowCount"] = ws.windowCount;
        list.append(map);
    }

    m_workspaces = list;
    emit workspacesChanged();
}

void WringController::refreshPopularApps()
{
    if (!m_backend) return;

    QList<ApplicationInfo> apps = m_backend->popularApplications();
    QVariantList list;

    for (const auto& app : apps) {
        QVariantMap map;
        map["name"] = app.name;
        map["displayName"] = app.displayName;
        map["executable"] = app.executable;
        map["desktopFile"] = app.desktopFile;
        map["popularity"] = app.popularity;

        if (!app.icon.isNull()) {
            map["icon"] = QVariant::fromValue(app.icon);
        }

        list.append(map);
    }

    m_popularApps = list;
    emit popularAppsChanged();
}

void WringController::selectNearestItem()
{
    int total = totalRing1Items();
    if (total == 0) {
        if (m_selectedIndex != -1) {
            m_selectedIndex = -1;
            emit selectedIndexChanged();
        }
        return;
    }

    double cursorAngle = angleFromCursor();

    int bestIndex = 0;
    double bestDist = 2.0 * M_PI;

    for (int i = 0; i < total; ++i) {
        double iAngle = itemAngle(i);
        double diff = qAbs(cursorAngle - iAngle);
        if (diff > M_PI) diff = 2.0 * M_PI - diff;

        if (diff < bestDist) {
            bestDist = diff;
            bestIndex = i;
        }
    }

    if (bestIndex != m_selectedIndex) {
        m_selectedIndex = bestIndex;
        emit selectedIndexChanged();
    }
}

void WringController::updateScreenGeometry()
{
    emit screenGeometryChanged();
}

int WringController::totalRing1Items() const
{
    return m_windows.size() + 1;
}

double WringController::itemAngle(int index) const
{
    int total = totalRing1Items();
    if (total == 0) return 0.0;

    double startAngle = -M_PI / 2.0;
    double angleStep = (2.0 * M_PI) / total;
    return startAngle + index * angleStep;
}

double WringController::angleFromCursor() const
{
    double dx = m_mouseX - m_ringCenterX;
    double dy = m_mouseY - m_ringCenterY;
    return qAtan2(dy, dx);
}
