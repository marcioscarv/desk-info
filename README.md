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
- **Lightweight** – Built with Tkinter + `psutil`, minimal CPU usage
- **Single Instance** – Prevents multiple copies using a Windows mutex

---

## Screenshot (Example Output)

<p align="center">
  <img src="https://i.imgur.com/ZyfixaL.png" alt="DeskInfo Screenshot" width="800"/>
  <br>
  <em>See an example</em>
</p>

---

## How It Works

1. Launches a **full-screen transparent Tkinter window**
2. Positions itself **behind all other windows** (like wallpaper)
3. Reads `config.cfg` as a **template**
4. Fills placeholders like `{cpu_usage}` with live data
5. Updates every `UPDATE_INTERVAL_MS` (default: 2000ms)

---

## Installation

### Requirements
- **Python 3.8+** ([Download](https://www.python.org/downloads/))
- **Windows** (primary target; Linux/macOS experimental)
- **psutil**:
  ```bash
  pip install psutil
  ```

### Quick Start
```bash
git clone https://github.com/marcioscarv/desk-info.git
cd desk-info
pip install psutil
python main.py
```

> First run generates `config.cfg` and `README.txt` automatically.

---

## Customization

### Edit `config.cfg`

Control everything with this simple text file:

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
- Reorder, remove, or add lines freely
- Mix text: `CPU: {cpu_usage} | RAM: {mem_used_gb}/{mem_total_gb}`
- Invalid variable? Shows as `{var}`

---

### Advanced Settings (`config.py`)

```python
UPDATE_INTERVAL_MS = 2000      # Refresh rate in ms
FONT_FAMILY = "Consolas"       # Font
FONT_SIZE = 11                 # Font size
GAP_X = 30                      # Left/right padding
GAP_Y = 30                      # Top/bottom padding
COLOR_DEFAULT = "white"        # Text color
COLOR_HIGHLIGHT = "dodgerblue" # Accent color
```

---

## Build Executable (.exe)

Create a standalone executable with **PyInstaller**:

```bash
pip install pyinstaller
pyinstaller --onefile --windowed --name=DeskInfo main.py
```

- Output: `dist/DeskInfo.exe`
- Size: ~10–15 MB
- No Python needed on target PC

---

## Limitations

- Primary monitor only (by default)
- High refresh rate may use more CPU on old PCs
- Linux/macOS: Experimental (Tkinter desktop layering issues)

---

## Contributing

Want to help?
- Add GPU monitoring
- Themes (dark/light)
- Multi-monitor support
- Cross-platform fixes

**Fork → Code → PR!**

---

## License

[MIT License](LICENSE) — free to use, modify, and share.

---

*Built with love for desktop power users. Questions? [Open an issue!](https://github.com/marcioscarv/desk-info/issues)*

> _Inspired by BGInfo, but **always live**._
