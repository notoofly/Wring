# Wring Architecture

## Overview

Wring uses a three-layer architecture with clear separation between UI, core logic, and platform integration.

```
┌─────────────────────────────┐
│          QML UI             │
│                             │
│ Wring                       │
│ ├── Ring 1 (windows)        │
│ ├── Ring 2 (workspaces)     │
│ └── Ring Items              │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│       Wring Core            │
│                             │
│ WringController             │
│ ├── State machine           │
│ ├── Window model            │
│ ├── Workspace model         │
│ └── Application model       │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│      Desktop Backend        │
├──────────────┬──────────────┤
│ Linux X11    │ Windows      │
│ Xlib/XCB     │ Win32 API    │
└──────────────┴──────────────┘
```

## Directory Structure

```
wring/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                    # Entry point
│   ├── core/
│   │   ├── types.hpp               # Core data types
│   │   ├── wring_controller.hpp    # Main controller (QML bridge)
│   │   └── wring_controller.cpp
│   ├── platform/
│   │   ├── desktop_backend.hpp     # Abstract backend interface
│   │   ├── linux/x11/
│   │   │   ├── x11_backend.*       # X11 implementation
│   │   │   ├── x11_window_manager.*
│   │   │   └── x11_input.*         # X11 global input
│   │   └── windows/
│   │       ├── windows_backend.*   # Win32 implementation
│   │       ├── windows_window_manager.*
│   │       └── windows_input.*
│   └── input/
│       ├── global_input.hpp        # Cross-platform input wrapper
│       └── global_input.cpp
├── qml/
│   ├── Main.qml                    # Root window
│   ├── Wring.qml                   # Main UI container
│   ├── Ring.qml                    # Reusable radial ring
│   ├── RingItem.qml                # Base ring item
│   ├── WindowItem.qml              # Window display
│   ├── Ring2Button.qml             # Ring 2 trigger
│   ├── WorkspaceItem.qml           # Workspace display
│   └── PopularAppItem.qml          # App launcher display
└── resources/
```

## Core Components

### WringController

Central controller exposed to QML. Manages:

- **State machine**: Hidden → Ring1 → Ring2
- **Window list**: Populated from backend
- **Workspace list**: Populated from backend
- **Popular apps**: Configured or discovered
- **Selection**: Angular-based nearest item selection
- **Ring 2 anchoring**: Centers on Ring 2 button position

### DesktopBackend (Interface)

Abstract interface for platform operations:

```cpp
class DesktopBackend {
    virtual QList<WindowInfo> listWindows() = 0;
    virtual bool activateWindow(PlatformWindowId id) = 0;
    virtual QPoint cursorPosition() = 0;
    virtual QList<WorkspaceInfo> listWorkspaces() = 0;
    virtual bool switchWorkspace(int index) = 0;
    virtual QList<ApplicationInfo> popularApplications() = 0;
    virtual bool launchApplication(const QString& app) = 0;
};
```

### GlobalInput

Platform-agnostic input handling. Detects:

- Super + Right Mouse Button (press/release)
- Cursor movement
- Mouse wheel

## Radial Layout Algorithm

Items are placed around a center point using polar coordinates:

```
angleStep = 2π / itemCount
startAngle = -π/2  (top of circle)

For item i:
  angle = startAngle + i × angleStep
  x = centerX + cos(angle) × radius
  y = centerY + sin(angle) × radius
```

## Selection Algorithm

Selection uses angular distance from cursor to each item:

```
cursorAngle = atan2(mouseY - centerY, mouseX - centerX)

For each item i:
  itemAngle = startAngle + i × angleStep
  distance = |cursorAngle - itemAngle|
  if distance > π: distance = 2π - distance

selectedItem = item with minimum distance
```

## State Transitions

```
Hidden ──[Super+RMB]──► Ring1
Ring1 ──[Ring2 Button]──► Ring2
Ring1 ──[Release]──► Activate window → Hidden
Ring2 ──[Workspace/App]──► Action → Hidden
Ring2 ──[Back]──► Ring1
Any ──[Escape/Right Click]──► Hidden
```

## Ring 2 Anchoring

Ring 2 center is computed from the Ring 2 button's position in Ring 1:

```
ring2ButtonAngle = startAngle + buttonIndex × ring1AngleStep
ring2ButtonX = ring1CenterX + cos(ring2ButtonAngle) × ring1Radius
ring2ButtonY = ring1CenterY + sin(ring2ButtonAngle) × ring1Radius

ring2Center = (ring2ButtonX, ring2ButtonY)
```

This ensures Ring 2 visually expands from the button that triggered it.

## Platform Details

### Linux X11

- Window enumeration via `XQueryTree` + `_NET_WM_NAME`
- Window activation via `_NET_ACTIVE_WINDOW` client message
- Global input via `XGrabButton` on root window
- Workspace detection via `_NET_NUMBER_OF_DESKTOPS`
- Icons via `_NET_WM_ICON` property

### Windows

- Window enumeration via `EnumWindows`
- Window activation via `SetForegroundWindow`
- Global input via `SetWindowsHookEx` (low-level mouse/keyboard hooks)
- Icons via `GetClassLongPtr(GCLP_HICON)` + `GetIconInfo`

## Error Handling

All platform operations return success/failure. The controller logs errors but never crashes. Missing features degrade gracefully (e.g., no workspace support shows only windows).

Logging categories:

- `wring.core` - Controller logic
- `wring.input` - Input handling
- `wring.platform.x11` - X11 backend
- `wring.platform.windows` - Windows backend
- `wring.qml` - QML interface

## Performance

- No polling loops for window list
- No QML reload on state change
- Backend queries only on demand or via native events
- GPU-accelerated rendering via Qt Quick
