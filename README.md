# DeskInfo

[![Python](https://img.shields.io/badge/Python-3.8%2B-blue)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Real-time system info overlay** — inspired by [BGInfo](https://learn.microsoft.com/en-us/sysinternals/downloads/bginfo), but **live, transparent, and glued to your wallpaper**.

DeskInfo displays essential system information **directly on your desktop background** in real time — no static images, no wallpaper changes. It runs as a transparent, always-on-top overlay that updates CPU, RAM, disk, network, and more every few seconds.

Perfect for developers, sysadmins, or anyone who wants a **live dashboard on their desktop**.

---

## Features

- **Real-Time Updates** – Refreshes every 2 seconds (configurable)
- **Transparent Overlay** – Stays behind all windows, ignores clicks
- **Fully Customizable** – Edit `config.cfg` to show exactly what you want
- **Lightweight** – Built with PyQt6 + `psutil`, minimal CPU usage
- **Single Instance** – Prevents multiple copies using a Windows mutex

---

## Screenshot (Example Output)

<p align="center">
  <img src="https://i.imgur.com/DNr8NNi.png" alt="DeskInfo Screenshot" width="800"/>
  <br>
  <em>See an example</em>
</p>

---

## How It Works

1. Launches a **full-screen transparent PyQt6 window**
2. Positions itself **behind all other windows** (like wallpaper)
3. Reads `config.cfg` as a **template**
4. Fills placeholders like `{cpu_usage}` with live data
5. Updates every `UPDATE_INTERVAL_MS` (default: 2000ms)

---

## Installation

### Requirements
- **Python 3.8+** ([Download](https://www.python.org/downloads/))
- **Windows** (primary target; Linux/macOS experimental)
- **Dependencies** (install via pip):
  ```bash
  pip install PyQt6 psutil
  ```
  - **PyQt6**: Handles the transparent GUI overlay (replaces Tkinter for better cross-platform compatibility and performance).
  - **psutil**: Provides system metrics like CPU, RAM, disk, and network info.

### Quick Start
```bash
git clone https://github.com/marcioscarv/desk-info.git
cd desk-info
pip install PyQt6 psutil
python desk-info.py
```

> First run generates `config.cfg` and `README.txt` automatically.  
> **Note**: On first launch, ensure your firewall/antivirus allows the app (it's safe, but new apps may trigger scans).

---

## Customization

### Edit `config.cfg`

Control everything with this simple text file (INI-like format for easy editing):

```ini
Machine Domain:   {machine_domain}
IP Address:       {ip_address}
{SEPARATOR}
User Name:        {user_name}
Logon Domain:     {logon_domain}
{SEPARATOR}
OS Version:       {os_version}
System Type:      {system_type}
{SEPARATOR}
CPU Usage:        {cpu_usage}
Memory:           {mem_used_gb} / {mem_total_gb}
Disk Usage (C:\): {disk_c_usage}
{disk_info}
```

#### Available Variables

| Variable           | Description                            | Example                   |
|--------------------|----------------------------------------|---------------------------|
| `{machine_domain}` | Domain or WORKGROUP                    | `WORKGROUP`               |
| `{ip_address}`     | Primary IPv4 address                   | `192.168.1.100`           |
| `{user_name}`      | Logged-in username                     | `john`                    |
| `{logon_domain}`   | user@DOMAIN                            | `john@CORP`               |
| `{os_version}`     | Windows version                        | `Windows 11 Pro`          |
| `{system_type}`    | Architecture                           | `AMD64`                   |
| `{cpu_usage}`      | CPU usage                              | `12%`                     |
| `{mem_used_gb}`    | Used memory                            | `3.2G`                    |
| `{mem_total_gb}`   | Total memory                           | `8.0G`                    |
| `{disk_c_usage}`   | C: drive usage %                       | `65.2%`                   |
| `{disk_info}`      | Free/total space for all drives        | `Free Space (C:): ...`    |
| `{SEPARATOR}`      | Horizontal line (50 dashes)            | `-------------------`     |

**Tips**:
- Reorder, remove, or add lines freely (each line becomes a displayed row).
- Mix text and variables: `CPU: {cpu_usage} | RAM: {mem_used_gb}/{mem_total_gb}`.
- Invalid variable? It shows as `{var}` (no crash).
- Supports multiline blocks for `{disk_info}` (e.g., per-drive details).
- Save and restart the app for changes to take effect.

### Advanced Settings (`config.py`)

Edit this Python file for deeper tweaks (restart required):

```python
UPDATE_INTERVAL_MS = 2000      # Refresh rate in ms (lower = faster, but higher CPU)
FONT_FAMILY = "Consolas"       # Font (monospace recommended for alignment)
FONT_SIZE = 11                 # Font size in points
GAP_X = 30                     # Horizontal padding from screen edges (pixels)
GAP_Y = 30                     # Vertical padding from screen edges (pixels)
COLOR_DEFAULT = "white"        # Default text color (Qt color name or hex)
COLOR_HIGHLIGHT = "dodgerblue" # Highlight/accent color for sections (e.g., separators)
TRANSPARENCY = 0.95            # Overlay opacity (0.0 = invisible, 1.0 = opaque)
ALWAYS_BEHIND = True           # Keep behind other windows (False for testing)
```

**Pro Tips**:
- Test transparency issues? Set `ALWAYS_BEHIND = False` temporarily.
- Custom fonts: Use any system font (e.g., "Courier New", "Segoe UI Mono").
- Colors: Use Qt names ("red", "#FF0000") or RGB tuples.

---

## Build Executable (.exe)

Create a standalone `.exe` with **PyInstaller** (no Python needed on target machines). This bundles PyQt6, psutil, and your icon.

### Requirements
- Install PyInstaller: `pip install pyinstaller`
- **Icon file**: Prepare a `.ico` file (e.g., `icon.ico` in the repo root). Use tools like [GIMP](https://www.gimp.org/) or online converters for PNG-to-ICO.

### Build Command
```bash
pyinstaller --onefile --windowed --name=DeskInfo --icon=icon.ico desk-info.py
```

#### Explanation of Flags
- `--onefile`: Packs everything into a single `.exe` (easier distribution, ~15–25 MB due to PyQt6).
- `--windowed`: Hides the console window (GUI-only mode).
- `--name=DeskInfo`: Sets the output executable name.
- `--icon=icon.ico`: Embeds your custom icon (app icon in taskbar/EXE properties). If no icon, omit this flag.

#### Output & Tips
- **Location**: `dist/DeskInfo.exe`
- **Size**: ~15–25 MB (PyQt6 is hefty; use `--exclude-module` for unused libs if needed).
- **Testing**: Run the `.exe` on a clean Windows machine (no Python required).
- **Advanced PyInstaller**: For smaller size, add a `.spec` file:
  ```python
  # desk-info.spec (generate with `pyi-makespec desk-info.py`)
  a = Analysis(['desk-info.py'], ...)
  a.datas += [('icon.ico', '.', 'DATA')]  # Include icon
  exe = EXE(a, ..., icon='icon.ico')
  ```
  Then build: `pyinstaller desk-info.spec`.
- **Troubleshooting**: If PyQt6 fails to bundle, ensure `pip install pyinstaller[hooks]` for better Qt support. Antivirus may flag the EXE—add exception.

---

## Limitations

- **Primary monitor only** (by default; multi-monitor via future PRs).
- **High refresh rate** (e.g., <500ms) may spike CPU on older hardware—monitor with Task Manager.
- **Linux/macOS**: Experimental (PyQt6 layering works better than Tkinter, but test desktop compositing; e.g., `QT_AUTO_SCREEN_SCALE_FACTOR=1` env var).
- **No GPU monitoring** yet (contribute? See below).
- **Icon embedding**: Custom icons require 256x256+ PNG source for best quality.

---

## Contributing

Want to help expand DeskInfo? We're open to ideas!

- **Add GPU monitoring** (e.g., via `nvidia-smi` or `GPUtil`).
- **Themes support** (dark/light mode, CSS-like styles).
- **Multi-monitor** (extend to secondary screens).
- **Cross-platform fixes** (better macOS Wayland support).
- **New variables** (e.g., `{battery_level}`, `{network_speed}`).

**How to Contribute**:
1. Fork the repo.
2. Create a feature branch (`git checkout -b feature/gpu-monitoring`).
3. Commit changes (`git commit -m "Add GPU usage variable"`).
4. Push (`git push origin feature/gpu-monitoring`).
5. Open a Pull Request!

See [CONTRIBUTING.md](CONTRIBUTING.md) for details (or create one!).

---

## License

[MIT License](LICENSE) — free to use, modify, and share. See [LICENSE](LICENSE) for full text.

---

*Built with love for desktop power users ❤️. Questions, bugs, or ideas? [Open an issue!](https://github.com/marcioscarv/desk-info/issues)*

> _Inspired by BGInfo, but **always live**._
