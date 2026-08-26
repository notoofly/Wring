# Wring

Cross-platform radial desktop window switcher and application launcher.

## Overview

Wring is a lightweight desktop utility that provides a radial menu for switching between open windows and launching applications. Press **Super + Right Mouse Button** anywhere on your desktop to activate Wring.

### Ring 1 - Window Switcher

Ring 1 appears at your cursor position and displays all currently open windows arranged in a circle. Move your cursor to highlight a window, then release to activate it.

### Ring 2 - Workspaces & Applications

Click the **+** button on Ring 1 to open Ring 2, which shows available workspaces and popular applications. Ring 2 appears centered on the Ring 2 button, not at your original cursor position.

### Mouse Wheel

Use the mouse wheel while Wring is open to cycle through items. Wraps around at both ends.

## Building

### Prerequisites

#### Linux (X11)

```bash
# Ubuntu/Debian
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev \
    libx11-dev libxext-dev libxcb1-dev libxcb-util-dev pkg-config

# Fedora (39+)
sudo dnf install gcc-c++ cmake ninja-build \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    libX11-devel libXext-devel xcb-util-devel pkgconf-pkg-config

# Rocky Linux / AlmaLinux / CentOS Stream 9+
sudo dnf install epel-release
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake ninja-build qt6-qtbase-devel qt6-qtdeclarative-devel \
    libX11-devel libXext-devel xcb-util-devel pkgconf-pkg-config

# openSUSE Tumbleweed
sudo zypper install -t pattern devel_basis
sudo zypper install cmake ninja gcc-c++ \
    qt6-base-devel qt6-declarative-devel \
    libX11-devel libXext-devel xcb-util-devel pkgconf-pkg-config

# openSUSE Leap 15.5+
sudo zypper install -t pattern devel_basis
sudo zypper install cmake ninja gcc-c++ \
    qt6-base-devel qt6-declarative-devel \
    libX11-devel libXext-devel xcb-util-devel pkgconf-pkg-config

# Arch / Manjaro
sudo pacman -S base-devel cmake ninja qt6-base qt6-declarative \
    libx11 libxext xcb-util pkgconf
```

#### Windows

- Visual Studio 2022 or later with C++ workload
- Qt 6.5+ installed via Qt Online Installer
- CMake 3.21+

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
cmake --build . -j$(nproc)
```

### Run

```bash
./build/wring
```

Press **Super + Right Mouse Button** to activate.

## Configuration

Wring uses Qt Settings. Configuration is stored in:

- Linux: `~/.config/wring/wring.conf`
- Windows: Registry under `HKEY_CURRENT_USER\Software\wring\wring`

### Available Settings

| Setting | Default | Description |
|---------|---------|-------------|
| ring1Radius | 150 | Radius of Ring 1 in pixels |
| ring2Radius | 120 | Radius of Ring 2 in pixels |
| itemSize | 64 | Size of each radial item |
| animationDuration | 200 | Animation duration in ms |
| showWindowTitle | true | Show window title below items |

## Architecture

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed architecture documentation.

## Platform Support

| Platform | Status |
|----------|--------|
| Linux X11 | Supported |
| Windows | Supported |
| Wayland | Not yet |
| macOS | Not yet |

## License

MIT
